module outputlogic #(
	parameter NUM_PIXELS = 160,
	parameter NUM_WEIGHTS = 400,
	parameter NUM_SAMPLES = 400
)(
	input wire clk,
	input wire rst_n,  // Asynchronous reset active low
	input wire enable,
	input wire clear,
	input wire [08:0] sample_iter,
	input wire [08:0] weight_iter,
	input wire [NUM_WEIGHTS-1:0][31:0] data_in,
	output wire [31:0] data_out,
	output wire done
);

	reg [NUM_WEIGHTS-1:0][31:0] Reg_R;
	reg [31:0] d_out;
	reg [08:0] iterator;
	reg [08:0] s_iter;
	reg flg_done;
	reg next_flg_done;

	assign Reg_R = data_in;
	assign data_out = d_out;
	assign iterator = weight_iter;
	assign done = flg_done;
	assign s_iter = sample_iter;

	always_ff @(posedge clk or negedge rst_n) begin
		if(~rst_n) begin
			d_out <= '{default: '0 };
			flg_done <= 0;
		end else if(enable) begin
			d_out <= Reg_R[iterator];
			flg_done <= next_flg_done;
		end else if(clear) begin
			flg_done <= 0;
			d_out <= '{default: '0 };
		end else begin
			flg_done <= next_flg_done;
		end
	end

	always_comb begin
		if(s_iter == NUM_SAMPLES-1) begin
			if(iterator == NUM_WEIGHTS-1) begin
				next_flg_done = 1;
			end else begin
				next_flg_done = 0;
			end
		end else begin
			next_flg_done = 0;
		end
	end

endmodule