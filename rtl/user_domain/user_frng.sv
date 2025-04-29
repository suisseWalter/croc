module user_frng (
  input  logic clk_i,
  input  logic rst_ni,

  input  sbr_obi_req_t obi_req_i,
  output sbr_obi_rsp_t obi_rsp_o
);

  // 32-bit LFSR
  logic [31:0] lfsr_q, lfsr_d;

  // LFSR feedback polynomial (x^32 + x^22 + x^2 + x + 1)
  always_comb begin
    lfsr_d = {lfsr_q[30:0], lfsr_q[31] ^ lfsr_q[21] ^ lfsr_q[1] ^ lfsr_q[0]};
  end

  // State register
  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (!rst_ni)
      lfsr_q <= 32'hACE1ACE1;
    else
      lfsr_q <= lfsr_d;
  end

  // OBI response logic
  always_comb begin
    obi_rsp_o.gnt    = obi_req_i.req;
    obi_rsp_o.rvalid = obi_req_i.req && !obi_req_i.we;
    obi_rsp_o.rdata  = lfsr_q;
    obi_rsp_o.err    = 1'b0;
  end

endmodule
