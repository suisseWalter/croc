module user_frng (
    input  logic clk_i,
    input  logic rst_ni,
    input  obi_pkg::obi_req_t  sbr_req_i,
    output obi_pkg::obi_resp_t sbr_resp_o
  );
  
    // Simple 32-bit LFSR-based PRNG
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
      sbr_resp_o.gnt    = sbr_req_i.req;
      sbr_resp_o.rvalid = sbr_req_i.req && !sbr_req_i.we;
      sbr_resp_o.rdata  = lfsr_q;
      sbr_resp_o.err    = 1'b0;
    end
  
  endmodule
  