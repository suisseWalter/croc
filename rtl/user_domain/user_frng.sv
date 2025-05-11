module user_frng import croc_pkg::*; (
  input  logic clk_i,
  input  logic rst_ni,

  input  sbr_obi_req_t obi_req_i,
  output sbr_obi_rsp_t obi_rsp_o
);

  // ============================================================================
  // 1. CORE DATA STRUCTURE EQUIVALENT
  // ============================================================================
  logic [31:0] status_0_q, status_0_d;
  logic [31:0] status_1_q, status_1_d;
  logic [31:0] status_2_q, status_2_d;
  logic [31:0] status_3_q, status_3_d;
  
  logic [31:0] mat1;  // Parameter for state transition
  logic [31:0] mat2;  // Parameter for state transition  
  logic [31:0] tmat;  // Parameter for tempering

  // ============================================================================
  // 2. KEY CONSTANTS
  // ============================================================================
  localparam TINYMT32_MASK = 32'h7fffffff;  // Mask for highest bit (31-bit mask)
  localparam TINYMT32_SH0  = 1;             // Shift constant 0
  localparam TINYMT32_SH1  = 10;            // Shift constant 1
  localparam TINYMT32_SH8  = 8;             // Shift constant 8

  // ============================================================================
  // 3. Interface Signals (OBI)
  // ============================================================================
  logic req_d, req_q;
  logic we_d, we_q;
  logic [SbrObiCfg.IdWidth-1:0] id_d, id_q;
  logic [31:0] rsp_data_d, rsp_data_q;
  logic rsp_err_d, rsp_err_q;
  
  // ============================================================================
  // 4. STATE TRANSITION FUNCTION
  // ============================================================================
  logic [31:0] x, y;
  logic [31:0] a, b;
  
  always_comb begin
    // Default: hold current state
    status_0_d = status_0_q;
    status_1_d = status_1_q;
    status_2_d = status_2_q;
    status_3_d = status_3_q;
    
    // Only update if there's a read request
    if (obi_req_i.req && !obi_req_i.a.we) begin
      y = status_3_q;
      x = (status_0_q & TINYMT32_MASK) ^ status_1_q ^ status_2_q;
      x = x ^ (x << TINYMT32_SH0);
      y = y ^ (y >> TINYMT32_SH0) ^ x;
      
      // State update
      status_0_d = status_1_q;
      status_1_d = status_2_q;
      status_2_d = x ^ (y << TINYMT32_SH1);
      status_3_d = y;
      a = (y & 1) ? mat1 : '0;
      b = (y & 1) ? mat2 : '0;
      
      status_1_d = status_1_d ^ a;
      status_2_d = status_2_d ^ b;
    end
  end

  // ============================================================================
  // 5. TEMPERING FUNCTION
  // ============================================================================
  logic [31:0] tempered_output;
  logic [31:0] t0, t1;
  
  always_comb begin
    t0 = status_3_q;
    t1 = status_0_q + (status_2_q >> TINYMT32_SH8);
    t0 = t0 ^ t1;
    
    if (t1 & 1) begin
      t0 = t0 ^ tmat;
    end
    
    tempered_output = t0;
  end

  // ============================================================================
  // 6. OBI INTERFACE LOGIC
  // ============================================================================
  // Capture request fields (1st stage)
  assign req_d = obi_req_i.req;
  assign we_d  = obi_req_i.a.we;
  assign id_d  = obi_req_i.a.aid;

  // Compute response (1st stage)
  always_comb begin
    rsp_data_d = '0;
    rsp_err_d  = '0;

    if (obi_req_i.req) begin
      if (!obi_req_i.a.we) begin
        rsp_data_d = tempered_output; // Return tempered output instead of raw state
      end else begin
        rsp_err_d = 1'b1; // Error on write operations
      end
    end
  end
  
  // ============================================================================
  // 7. REGISTER UPDATES
  // ============================================================================
  // Response registers (2nd stage)
  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (!rst_ni) begin
      req_q       <= '0;
      we_q        <= '0;
      id_q        <= '0;
      rsp_data_q  <= '0;
      rsp_err_q   <= '0;
    end else begin
      req_q       <= req_d;
      we_q        <= we_d;
      id_q        <= id_d;
      rsp_data_q  <= rsp_data_d;
      rsp_err_q   <= rsp_err_d;
    end
  end

  // TinyMT32 state registers
  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (!rst_ni) begin
      // TODO -  Parameters - if possible, adding some switching of these at restart would be favorable
      status_0_q <= 32'h7B78CF44;  
      status_1_q <= 32'h8F7011EE;  
      status_2_q <= 32'hFC78FF1F;  
      status_3_q <= 32'h3793FDFF;  
      
      //TODO -  maybe keep a list, use as index the last command passed before reset or smthing?
      mat1 <= 32'h8F7011EE;
      mat2 <= 32'hFC78FF1F;
      tmat <= 32'h3793FDFF;
    end else begin
      status_0_q <= status_0_d;
      status_1_q <= status_1_d;
      status_2_q <= status_2_d;
      status_3_q <= status_3_d;
    end
  end

  // ============================================================================
  // 8. OBI RESPONSE OUTPUT ASSIGNMENTS
  // ============================================================================
  // Wire the response
  assign obi_rsp_o.gnt            = obi_req_i.req; // Grant signal immediately
  assign obi_rsp_o.rvalid         = req_q;         // Valid response after 2 cycles
  assign obi_rsp_o.r.rdata        = rsp_data_q;
  assign obi_rsp_o.r.rid          = id_q;
  assign obi_rsp_o.r.err          = rsp_err_q;
  assign obi_rsp_o.r.r_optional   = '0;

endmodule