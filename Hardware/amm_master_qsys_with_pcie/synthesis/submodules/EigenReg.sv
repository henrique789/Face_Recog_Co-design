module EigenReg #(
	parameter NUM_PIXELS = 161,
	parameter COLS_SIZE = 8
)
(
	input wire clk,
	input wire rst_n,  // Asynchronous reset active low
	input wire enable,
	input wire clear,
	input wire [15:0] pixel_iter,
	input wire [03:0] eigen_iter,
	input wire [31:0] data_in,
	output wire [COLS_SIZE-1:0][NUM_PIXELS-1:0][31:0] data_out
);

	reg [COLS_SIZE-1:0][NUM_PIXELS-1:0][31:0] EigReg;
	reg [15:0] iterator_row;
	reg [03:0] iterator_col;

	assign data_out = EigReg;
	assign iterator_row = pixel_iter;
	assign iterator_col = eigen_iter;

	always_ff @(posedge clk or negedge rst_n) begin
		if(~rst_n) begin
			EigReg <= '{default: '0 };
		end else if (enable) begin
			EigReg[iterator_col][iterator_row] <= data_in;
		end else if (clear) begin
			EigReg <= '{default: '0 };
		end
	end

endmodule // EigenReg