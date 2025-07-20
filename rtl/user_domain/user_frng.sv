module user_frng import croc_pkg::*; (
  input  logic            clk_i,
  input  logic            rst_ni,
  input  sbr_obi_req_t    obi_req_i,
  output sbr_obi_rsp_t    obi_rsp_o
);

  // ============================================================================
  // 1. CORE DATA STRUCTURE
  // ============================================================================
  logic [31:0] status_0_q, status_0_d;
  logic [31:0] status_1_q, status_1_d;
  logic [31:0] status_2_q, status_2_d;
  logic [31:0] status_3_q, status_3_d;
  logic [31:0] mat1, mat2, tmat;

  // ============================================================================
  // 2. KEY CONSTANTS
  // ============================================================================
  localparam TINYMT32_MASK = 32'h7fff_ffff;
  localparam TINYMT32_SH0  = 1;
  localparam TINYMT32_SH1  = 10;
  localparam TINYMT32_SH8  = 8;
  localparam SEED_WORDS    = 256;

  // ============================================================================
  // 3. INTERNAL SEED SRAM (READ-ONLY)
  // ============================================================================
  logic         sram_req;
  logic         sram_we;
  logic [1:0]   sram_addr;
  logic [31:0]  sram_wdata;
  logic [31:0]  sram_rdata;

  assign sram_we    = 1'b0;
  assign sram_wdata = 32'd0;

  tc_sram_impl #(
    .NumWords   (SEED_WORDS),
    .DataWidth  (64),
    .ByteWidth  (8),
    .NumPorts   (1),
    .Latency    (1),
    .PrintSimCfg(0)
  ) seed_sram (
    .clk_i   (clk_i),
    .rst_ni  (rst_ni),
    .impl_i  (1'b0),
    .impl_o  (),
    .req_i   ({sram_req}),
    .we_i    ({sram_we}),
    .addr_i  ({sram_addr}),
    .wdata_i ({sram_wdata}),
    .be_i    ({{4{{1'b1}}}}),
    .rdata_o ({sram_rdata})
  );

  // ============================================================================
  // 4. SEEDING FSM COMBINATIONAL
  // ============================================================================
  typedef enum logic [1:0] { IDLE, READ, WAIT, DONE } seed_st_e;
  seed_st_e     seed_st_q, seed_st_d;
  logic [1:0]   seed_cnt_q, seed_cnt_d;

  always_comb begin
    seed_st_d  = seed_st_q;
    seed_cnt_d = seed_cnt_q;
    sram_req   = 1'b0;
    sram_addr  = seed_cnt_q;
    case (seed_st_q)
      IDLE:       seed_st_d = READ;
      READ: begin
        sram_req  = 1'b1;
        seed_st_d = WAIT;
      end
      WAIT: begin
        if (seed_cnt_q == 4-1)
          seed_st_d = DONE;
        else begin
          seed_st_d  = READ;
          seed_cnt_d = seed_cnt_q + 1;
        end
      end
      DONE:      seed_st_d = DONE;
    endcase
  end

  // ============================================================================
  // 5. MANUAL SEEDING LOGIC
  // ============================================================================
  // mini reg storage: 4 words of 32 bits & reseed trigger as a 5th word
  logic [SbrObiCfg.DataWidth-1:0] mem [0:4];
  logic manual_seed_trigger;
  
  assign manual_seed_trigger = (mem[4] == 32'hFFFF_FFFF);

  // ============================================================================
  // 6. SYNC: SEEDING FSM + STATE REGISTER UPDATE
  // ============================================================================
  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (!rst_ni) begin
      seed_st_q   <= IDLE;
      seed_cnt_q  <= 2'd0;
      status_0_q  <= 32'hDEAD_BEEF;
      status_1_q  <= 32'hDEAD_BEEF;
      status_2_q  <= 32'hDEAD_BEEF;
      status_3_q  <= 32'hDEAD_BEEF;
      mat1        <= 32'h8F70_11EE;
      mat2        <= 32'hFC78_FF1F;
      tmat        <= 32'h3793_FDFF;
    end else begin
      // advance FSM
      seed_st_q  <= seed_st_d;
      seed_cnt_q <= seed_cnt_d;

      // Check for manual seeding trigger
      if (manual_seed_trigger) begin
        // Overwrite status registers with mem[0] through mem[3]
        status_0_q <= mem[0];
        status_1_q <= mem[1];
        status_2_q <= mem[2];
        status_3_q <= mem[3];
        // Clear the trigger
        mem[4] <= 32'h0000_0000;
      end else if (seed_st_q == WAIT) begin
        // load seed words into state
        unique case (seed_cnt_q)
          2'd0: status_0_q <= 32'h7B78CF44 ^ sram_rdata;
          2'd1: status_1_q <= 32'h8F7011EE ^ sram_rdata;
          2'd2: status_2_q <= 32'hFC78FF1F ^ sram_rdata;
          2'd3: status_3_q <= 32'h3793FDFF ^ sram_rdata;
        endcase
      end else if (seed_st_q == DONE) begin
        // normal TinyMT state update
        status_0_q <= status_0_d;
        status_1_q <= status_1_d;
        status_2_q <= status_2_d;
        status_3_q <= status_3_d;
      end
    end
  end

  // ============================================================================
  // 7. STATE TRANSITION & TEMPERING
  // ============================================================================
  logic         seeded;
  logic [31:0]  x, y, a, b;
  logic [31:0]  t0, t1, tempered_output;

  assign seeded = (seed_st_q == DONE);

  // next-state combinational
  always_comb begin
    status_0_d = status_0_q;
    status_1_d = status_1_q;
    status_2_d = status_2_q;
    status_3_d = status_3_q;

    if (seeded && req_q && !we_q) begin
      y = status_3_q;
      x = (status_0_q & TINYMT32_MASK) ^ status_1_q ^ status_2_q;
      x = x ^ (x << TINYMT32_SH0);
      y = y ^ (y >> TINYMT32_SH0) ^ x;

      status_0_d = status_1_q;
      status_1_d = status_2_q;
      status_2_d = x ^ (y << TINYMT32_SH1);
      status_3_d = y;

      a = y[0] ? mat1 : 32'd0;
      b = y[0] ? mat2 : 32'd0;
      status_1_d = status_1_d ^ a;
      status_2_d = status_2_d ^ b;
    end
  end

  // tempering
  always_comb begin
    t0 = status_3_q;
    t1 = status_0_q + (status_2_q >> TINYMT32_SH8);
    t0 = t0 ^ t1;
    if (t1[0]) t0 = t0 ^ tmat;
    tempered_output = t0;
  end

  // ============================================================================
  // 8. OBI INTERFACE LOGIC
  // ============================================================================
  logic                     req_d, req_q;
  logic                     we_d,  we_q;
  logic [SbrObiCfg.IdWidth-1:0] id_d, id_q;
  logic [31:0]              rsp_data_d, rsp_data_q;
  logic                     rsp_err_d, rsp_err_q;
  logic [31:0]              wdata_q, data_d;
  logic [SbrObiCfg.AddrWidth-1:0] addr_q;
  logic [SbrObiCfg.AddrWidth-1:0] addr_d;

  // capture request
  assign req_d = obi_req_i.req;
  assign we_d  = obi_req_i.a.we;
  assign id_d  = obi_req_i.a.aid;
  assign data_d = obi_req_i.a.wdata;
  assign addr_d = obi_req_i.a.addr;

  // compute response
  always_comb begin
    rsp_data_d = 32'd0;
    rsp_err_d  = 1'b0;
    if (req_d) begin
      if (!we_d)
        rsp_data_d = tempered_output;
      else
        rsp_data_d  =32'd0;
    end
  end

  // stage2 registers
  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (!rst_ni) begin
      req_q      <= 1'b0;
      we_q       <= 1'b0;
      id_q       <= '0;
      rsp_data_q <= 32'd0;
      rsp_err_q  <= 1'b0;
      wdata_q    <= 32'd0;
      addr_q     <= '0;
    end else begin
      req_q      <= req_d;
      we_q       <= we_d;
      id_q       <= id_d;
      rsp_data_q <= rsp_data_d;
      rsp_err_q  <= rsp_err_d;
      wdata_q    <= data_d;
      addr_q     <= addr_d;
    end
  end

  // ============================================================================
  // 9. MEMORY BANK LOGIC
  // ============================================================================
  // Read & write logic
  logic [2:0] word_addr;
  logic [SbrObiCfg.DataWidth-1:0] rsp_data;
  logic rsp_err;

  assign word_addr = addr_q[4:2];

  always_ff @(posedge clk_i) begin
    if (req_q && we_q && word_addr < 5)
      mem[word_addr] <= wdata_q;
  end

  always_comb begin
    rsp_data = '0;
    rsp_err  = '0;

    if (req_q) begin
      if (word_addr >= 8) begin
        rsp_err = 1'b1;
      end else if (!we_q) begin
        rsp_data = mem[word_addr];
      end
    end
  end

  // OBI response outputs
  assign obi_rsp_o.gnt          = req_d;
  assign obi_rsp_o.rvalid       = req_q;
  assign obi_rsp_o.r.rdata      = rsp_data_q;
  assign obi_rsp_o.r.rid        = id_q;
  assign obi_rsp_o.r.err        = rsp_err_q;
  assign obi_rsp_o.r.r_optional = '0;

endmodule