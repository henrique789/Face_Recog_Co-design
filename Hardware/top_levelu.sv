module top_levelu #(
	parameter NUM_PIXELS = 161,
	parameter NUM_WEIGHTS = 400,
	parameter COLS_SIZE = 8,
	parameter NUM_SAMPLES = 400
)(
	//input
	input wire clk,
	input wire rst_n,
	input wire enable_face,
	input wire enable_mean,
	input wire enable_p,
	input wire enable_r,
	input wire clear_p,
	input wire clear_r,
	input wire clear_f,
	input wire clear_m,
	input wire clear_out_logic,
	input wire enable_out_logic,
	input wire [03:0] eigen_iter,
	input wire [08:0] weight_iter,
	input wire [08:0] sample_iter,
	input wire [15:0] pixel_iter,
	input wire [31:0]data_in,
	// output
	output wire done_flg,
	output wire [31:0]data_out
  );

	reg [NUM_PIXELS-1:0][31:0]f_out;
	reg [NUM_PIXELS-1:0][31:0]m_out;
	reg [COLS_SIZE-1:0][NUM_PIXELS-1:0][31:0] p_out;
	reg [NUM_WEIGHTS-1:0][31:0] r_out;

	FPreg #(
		.NUM_PIXELS(NUM_PIXELS)
		)RegM(
			.clk(clk),
			.rst_n(rst_n),
			.enable(enable_mean),
			.clear(clear_m),
			.data_in(data_in),
			.data_out(m_out)
	);

	FPreg #(
		.NUM_PIXELS(NUM_PIXELS)
		)RegF(
			.clk(clk),
			.rst_n(rst_n),
			.enable(enable_face),
			.clear(clear_f),
			.data_in(data_in),
			.data_out(f_out)
	);

	EigenReg #(
		.NUM_PIXELS(NUM_PIXELS),
		.COLS_SIZE(COLS_SIZE)
		)RegEig(
			.clk(clk),
			.rst_n(rst_n),
			.enable(enable_p),
			.clear(clear_p),
			.pixel_iter(pixel_iter),
			.eigen_iter(eigen_iter),
			.data_in(data_in),
			.data_out(p_out)
	);

	ProjReg #(
		.NUM_PIXELS(NUM_PIXELS),
		.NUM_WEIGHTS(NUM_WEIGHTS),
		.COLS_SIZE(COLS_SIZE)
		)RegProj(
			.clk(clk),
			.rst_n(rst_n),
			.enable(enable_r),
			.clear(clear_r),
			.pixel_iter(pixel_iter),
			.weight_iter(weight_iter),
			.f_in(f_out),
			.m_in(m_out),
			.p_in(p_out),
			.data_out(r_out)
	);

	outputlogic #(
		.NUM_PIXELS(NUM_PIXELS),
		.NUM_WEIGHTS(NUM_WEIGHTS),
		.NUM_SAMPLES(NUM_SAMPLES)
		)OUT_LOGIC(
			// input
			.clk(clk),
			.rst_n(rst_n),
			.enable(enable_out_logic),
			.clear(clear_out_logic),
			.sample_iter(sample_iter),
			.weight_iter(weight_iter),
			.data_in(r_out),
			// output
			.data_out(data_out),
			.done(done_flg)
	);

endmodule