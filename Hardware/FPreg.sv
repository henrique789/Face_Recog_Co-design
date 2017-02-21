module FPreg #(
	parameter NUM_PIXELS = 161
)
(
	input wire clk,
	input wire rst_n,  // Asynchronous reset active low
	input wire enable,
	input wire clear,
	input wire [31:0] data_in,
	output wire [NUM_PIXELS-1:0][31:0] data_out
);

	reg [NUM_PIXELS-1:0][31:0] regFP;

	assign data_out = regFP;

	always_ff @(posedge clk or negedge rst_n) begin
		if(~rst_n) begin
			regFP <= '{default: '0 };
		end else if (enable) begin
			regFP[31] <= data_in;
			regFP[30] <= regFP[31];
			regFP[29] <= regFP[30];
			regFP[28] <= regFP[29];
			regFP[27] <= regFP[28];
			regFP[26] <= regFP[27];
			regFP[25] <= regFP[26];
			regFP[24] <= regFP[25];
			regFP[23] <= regFP[24];
			regFP[22] <= regFP[23];
			regFP[21] <= regFP[22];
			regFP[20] <= regFP[21];
			regFP[19] <= regFP[20];
			regFP[18] <= regFP[19];
			regFP[17] <= regFP[18];
			regFP[16] <= regFP[17];
			regFP[15] <= regFP[16];
			regFP[14] <= regFP[15];
			regFP[13] <= regFP[14];
			regFP[12] <= regFP[13];
			regFP[11] <= regFP[12];
			regFP[10] <= regFP[11];
			regFP[9] <= regFP[10];
			regFP[8] <= regFP[9];
			regFP[7] <= regFP[8];
			regFP[6] <= regFP[7];
			regFP[5] <= regFP[6];
			regFP[4] <= regFP[5];
			regFP[3] <= regFP[4];
			regFP[2] <= regFP[3];
			regFP[1] <= regFP[2];
			regFP[0] <= regFP[1];
		end else if (clear) begin
			regFP <= '{default: '0 };
		end else begin
			regFP[31] <= regFP[31];
			regFP[30] <= regFP[30];
			regFP[29] <= regFP[29];
			regFP[28] <= regFP[28];
			regFP[27] <= regFP[27];
			regFP[26] <= regFP[26];
			regFP[25] <= regFP[25];
			regFP[24] <= regFP[24];
			regFP[23] <= regFP[23];
			regFP[22] <= regFP[22];
			regFP[21] <= regFP[21];
			regFP[20] <= regFP[20];
			regFP[19] <= regFP[19];
			regFP[18] <= regFP[18];
			regFP[17] <= regFP[17];
			regFP[16] <= regFP[16];
			regFP[15] <= regFP[15];
			regFP[14] <= regFP[14];
			regFP[13] <= regFP[13];
			regFP[12] <= regFP[12];
			regFP[11] <= regFP[11];
			regFP[10] <= regFP[10];
			regFP[9] <= regFP[9];
			regFP[8] <= regFP[8];
			regFP[7] <= regFP[7];
			regFP[6] <= regFP[6];
			regFP[5] <= regFP[5];
			regFP[4] <= regFP[4];
			regFP[3] <= regFP[3];
			regFP[2] <= regFP[2];
			regFP[1] <= regFP[1];
			regFP[0] <= regFP[0];
		end
	end

endmodule // FPreg