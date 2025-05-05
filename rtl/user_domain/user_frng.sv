module user_frng import croc_pkg::*; (
  input  logic clk_i,
  input  logic rst_ni,

  input  sbr_obi_req_t obi_req_i,
  output sbr_obi_rsp_t obi_rsp_o
);

  // 32-bit LFSR
  logic [31:0] lfsr_q, lfsr_d;

  // 2-cycle response registers
  logic req_d, req_q; // Request valid
  logic we_d, we_q;   // Write enable
  logic [SbrObiCfg.IdWidth-1:0] id_d, id_q; // Request ID
  logic [31:0] rsp_data_d, rsp_data_q; // Response data
  logic rsp_err_d, rsp_err_q;

  // LFSR feedback polynomial (x^32 + x^22 + x^2 + x + 1)
  always_comb begin
    lfsr_d = lfsr_q; // Hold by default
    if (obi_req_i.req && !obi_req_i.a.we)
      lfsr_d = {lfsr_q[30:0], lfsr_q[31] ^ lfsr_q[21] ^ lfsr_q[1] ^ lfsr_q[0]};
  end

  // State register
  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (!rst_ni)
      lfsr_q <= 32'hACE1ACE1; // Initial seed
    else
      lfsr_q <= lfsr_d;
  end

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
        rsp_data_d = lfsr_q;
      end else begin
        rsp_err_d = 1'b1;
      end
    end
  end

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

  // Wire the response
  assign obi_rsp_o.gnt            = obi_req_i.req; // Grant signal immediately
  assign obi_rsp_o.rvalid         = req_q;              // Valid response after 2 cycles
  assign obi_rsp_o.r.rdata        = rsp_data_q;
  assign obi_rsp_o.r.rid          = id_q;
  assign obi_rsp_o.r.err          = rsp_err_q;
  assign obi_rsp_o.r.r_optional   = '0;

endmodule