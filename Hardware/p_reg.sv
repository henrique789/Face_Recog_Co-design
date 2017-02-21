module p_reg #(
	parameter NUM_PIXELS = 160,
	parameter COLS_SIZE = 8
)
(
	input wire clk,
	input wire rst_n,  // Asynchronous reset active low
	input wire enable,
	input wire clear,
	input wire [15:0] pixel_iter,
	input wire [3:0] eigen_iter,
	input wire [31:0] data_in,
	output wire [COLS_SIZE-1:0][NUM_PIXELS-1:0][7:0] data_out
);

	reg [COLS_SIZE-1:0][NUM_PIXELS-1:0][7:0] Reg_P;
	reg [15:0] iterator_row;
	reg [3:0] iterator_col;

	assign data_out = Reg_P;
	assign iterator_row = pixel_iter;
	assign iterator_col = eigen_iter;

	always_ff @(posedge clk or negedge rst_n) begin
		if(~rst_n) begin
			Reg_P <= '{default: '0 };
		end else if (enable) begin
			Reg_P[iterator_col][iterator_row] <= data_in[7:0];
			Reg_P[iterator_col][iterator_row+1] <= data_in[15:8];
			Reg_P[iterator_col][iterator_row+2] <= data_in[23:16];
			Reg_P[iterator_col][iterator_row+3] <= data_in[31:24];
		end else if (clear) begin
			Reg_P <= '{default: '0 };
		end
	end




endmodule