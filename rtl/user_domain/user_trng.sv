module user_trng import croc_pkg::*; (
  input logic clk_i,
  input logic rst_ni,
  input sbr_obi_req_t obi_req_i,
  output sbr_obi_rsp_t obi_rsp_o
);

  // Ring oscillator parameters - I think we'll have to increase this to get it to work on hardware, TBD
  localparam int NO_STAGES = 9;    
  
  // Ring oscillator signals - everythign like in our simplest found example
  logic enable_osc;
  logic osc_out;
  logic [31:0] random_value;
  

  // Internal wires for the ring
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
  
  // Sampling logic - capture the oscillator output into a shift register
  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (!rst_ni) begin
      random_value <= 32'hACE1ACE1; // Initial seed
      enable_osc <= 1'b1;          // Enable oscillator on reset
    end else begin
      // Shift in the oscillator output
      random_value <= {random_value[30:0], osc_out};
      enable_osc <= 1'b1;          // Keep oscillator running
    end
  end

  // Registers to hold request fields
  logic req_d, req_q;               // Request valid
  logic we_d, we_q;                 // Write enable
  logic [SbrObiCfg.IdWidth-1:0] id_d, id_q; // Request ID

  // Capture request fields
  assign req_d = obi_req_i.req;
  assign we_d = obi_req_i.a.we;
  assign id_d = obi_req_i.a.aid;
  
  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (!rst_ni) begin
      req_q <= '0;
      we_q <= '0;
      id_q <= '0;
    end else begin
      req_q <= req_d;
      we_q <= we_d;
      id_q <= id_d;
    end
  end

  // Response logic
  logic [SbrObiCfg.DataWidth-1:0] rsp_data;
  logic rsp_err;
  
  always_comb begin
    rsp_data = '0;
    rsp_err = '0;
    if (req_q) begin
      if (!we_q) begin
        rsp_data = random_value; // Return the random value from ring oscillator
      end else begin
        rsp_err = 1'b1;          // Writes are not supported
      end
    end
  end

  // Wire the response
  assign obi_rsp_o.gnt = obi_req_i.req;   // Grant signal
  assign obi_rsp_o.r.rdata = rsp_data;    // Response data
  assign obi_rsp_o.r.rid = id_q;          // Response ID
  assign obi_rsp_o.r.err = rsp_err;       // Response error
  assign obi_rsp_o.r.r_optional = '0;     // Optional fields (if any)
endmodule