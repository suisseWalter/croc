module user_frng import croc_pkg::*; (
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
      lfsr_q <= 32'hACE1ACE1; // Initial seed
    else
      lfsr_q <= lfsr_d;
  end

  // Registers to hold request fields
  logic req_d, req_q; // Request valid
  logic we_d, we_q;   // Write enable
  logic [SbrObiCfg.IdWidth-1:0] id_d, id_q; // Request ID

  // Capture request fields
  assign req_d = obi_req_i.req;
  assign we_d  = obi_req_i.a.we;
  assign id_d  = obi_req_i.a.aid;

  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (!rst_ni) begin
      req_q <= '0;
      we_q  <= '0;
      id_q  <= '0;
    end else begin
      req_q <= req_d;
      we_q  <= we_d;
      id_q  <= id_d;
    end
  end

  // Response logic
  logic [SbrObiCfg.DataWidth-1:0] rsp_data;
  logic rsp_err;

  always_comb begin
    rsp_data = '0;
    rsp_err  = '0;

    if (req_q) begin
      if (!we_q) begin
        rsp_data = lfsr_q; // Return the LFSR value
      end else begin
        rsp_err = 1'b1; // Writes are not supported
      end
    end
  end

  // Wire the response
  assign obi_rsp_o.gnt       = obi_req_i.req; // Grant signal
  assign obi_rsp_o.r.rdata   = rsp_data;      // Response data
  assign obi_rsp_o.r.rid     = id_q;          // Response ID
  assign obi_rsp_o.r.err     = rsp_err;       // Response error
  assign obi_rsp_o.r.r_optional = '0;         // Optional fields (if any)

endmodule
