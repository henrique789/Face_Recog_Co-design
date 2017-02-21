module registerx32 #(
	parameter NUM_PIXELS = 160,
	parameter NUM_WEIGHTS = 240,
	parameter COLS_SIZE = 8
)
(
	input wire clk,
	input wire rst_n,  // Asynchronous reset active low
	input wire enable,
	input wire clear,
	input wire [15:0] pixel_iter,
	input wire [7:0] weight_iter,
	input wire [NUM_PIXELS-1:0][8:0] f_in,
	input wire [NUM_PIXELS-1:0][8:0] m_in,
	input wire [COLS_SIZE-1:0][NUM_PIXELS-1:0][7:0] p_in,
	output wire [NUM_WEIGHTS-1:0][31:0] data_out
);

	reg [COLS_SIZE-1:0][NUM_PIXELS-1:0][7:0] Reg_P;
	reg [NUM_PIXELS-1:0][8:0] Reg_F;
	reg [NUM_PIXELS-1:0][8:0] Reg_M;
	reg [NUM_WEIGHTS-1:0][31:0] Reg_R;
	reg [15:0] iterator_row;
	reg [7:0] iterator_wei;

	assign data_out = Reg_R;
	assign Reg_P = p_in;
	assign Reg_F = f_in;
	assign Reg_M = m_in;
	assign iterator_row = pixel_iter;
	assign iterator_wei = weight_iter;

	always_ff @(posedge clk or negedge rst_n) begin
		if(~rst_n) begin
			Reg_R <= '{default: '0 };
		end else if (enable) begin
			Reg_R[iterator_wei] <= Reg_R[iterator_wei] + (Reg_P[0][iterator_row]*(Reg_F[iterator_row]-Reg_M[iterator_row]));
			Reg_R[iterator_wei+1] <= Reg_R[iterator_wei+1] + (Reg_P[1][iterator_row]*(Reg_F[iterator_row]-Reg_M[iterator_row]));
			Reg_R[iterator_wei+2] <= Reg_R[iterator_wei+2] + (Reg_P[2][iterator_row]*(Reg_F[iterator_row]-Reg_M[iterator_row]));
			Reg_R[iterator_wei+3] <= Reg_R[iterator_wei+3] + (Reg_P[3][iterator_row]*(Reg_F[iterator_row]-Reg_M[iterator_row]));
			Reg_R[iterator_wei+4] <= Reg_R[iterator_wei+4] + (Reg_P[4][iterator_row]*(Reg_F[iterator_row]-Reg_M[iterator_row]));
			Reg_R[iterator_wei+5] <= Reg_R[iterator_wei+5] + (Reg_P[5][iterator_row]*(Reg_F[iterator_row]-Reg_M[iterator_row]));
			Reg_R[iterator_wei+6] <= Reg_R[iterator_wei+6] + (Reg_P[6][iterator_row]*(Reg_F[iterator_row]-Reg_M[iterator_row]));
			Reg_R[iterator_wei+7] <= Reg_R[iterator_wei+7] + (Reg_P[7][iterator_row]*(Reg_F[iterator_row]-Reg_M[iterator_row]));
		end else if (clear) begin
			Reg_R <= '{default: '0 };
		end
	end
	
endmodule // registerx32