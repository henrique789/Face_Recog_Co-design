module regx8 #(
	parameter NUM_PIXELS = 160
)
(
	input wire clk,
	input wire rst_n,  // Asynchronous reset active low
	input wire clean,
	input wire enable,
	input wire [15:0] iterator,
	input wire [31:0] data_in,
	output wire [NUM_PIXELS-1:0][8:0] data_out
);


	reg [NUM_PIXELS-1:0][8:0] Reg;
	reg [15:0] iter;

	assign data_out = Reg;
	assign iter = iterator;

	always_ff @(posedge clk or negedge rst_n) begin
		if(~rst_n) begin
			Reg <= '{default: '0 };
		end else if (enable) begin
			Reg[iter] <= data_in[7:0];
			Reg[iter+1] <= data_in[15:8];
			Reg[iter+2] <= data_in[23:16];
			Reg[iter+3] <= data_in[31:24];
		end else if (clean) begin
			Reg <= '{default: '0 };
		end
	end
endmodule