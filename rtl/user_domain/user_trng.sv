module user_trng import croc_pkg::*; (
  input logic clk_i,
  input logic rst_ni,

  input sbr_obi_req_t obi_req_i,
  output sbr_obi_rsp_t obi_rsp_o
);

  localparam int NO_STAGES = 9;    

  // Ring oscillator signals
  logic enable_osc;
  logic osc_out;
  logic [31:0] random_value;

  logic [NO_STAGES:0] wi;

  // First stage gets feedback from last stage
  assign wi[0] = enable_osc ? wi[NO_STAGES] : 1'b0;

  // Generate the ring of inverters
  genvar i;
  generate
    for(i = 0; i < NO_STAGES; i = i+1) begin
      not (wi[i+1], wi[i]);
    end
  endgenerate

  assign osc_out = enable_osc ? wi[NO_STAGES] : 1'b0;

  // Sampling 
  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (!rst_ni) begin
      random_value <= 32'hACE1ACE1;
      enable_osc   <= 1'b1;
    end else begin
      random_value <= {random_value[30:0], osc_out};
      enable_osc   <= 1'b1;
    end
  end

  // as per testing in FRNG, 2-cycle response registers
  logic req_d, req_q;
  logic we_d, we_q;
  logic [SbrObiCfg.IdWidth-1:0] id_d, id_q;
  logic [SbrObiCfg.DataWidth-1:0] rsp_data_d, rsp_data_q;
  logic rsp_err_d, rsp_err_q;

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
        rsp_data_d = random_value;
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
  assign obi_rsp_o.gnt          = obi_req_i.req;
  assign obi_rsp_o.rvalid       = req_q;
  assign obi_rsp_o.r.rdata      = rsp_data_q;
  assign obi_rsp_o.r.rid        = id_q;
  assign obi_rsp_o.r.err        = rsp_err_q;
  assign obi_rsp_o.r.r_optional = '0;

endmodule
