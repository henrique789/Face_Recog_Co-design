module ProjReg #(
	parameter NUM_PIXELS = 161,
	parameter NUM_WEIGHTS = 400,
	parameter COLS_SIZE = 8
)
(
	input wire clk,
	input wire rst_n,  // Asynchronous reset active low
	input wire enable,
	input wire clear,
	input wire [15:0] pixel_iter,
	input wire [08:0] weight_iter,
	input wire [NUM_PIXELS-1:0][31:0] f_in,
	input wire [NUM_PIXELS-1:0][31:0] m_in,
	input wire [COLS_SIZE-1:0][NUM_PIXELS-1:0][31:0] p_in,
	output wire [NUM_WEIGHTS-1:0][31:0] data_out
);

	reg [COLS_SIZE-1:0][NUM_PIXELS-1:0][31:0] Reg_P;
		reg [31:0] RegP0;
		reg [31:0] RegP1;
		reg [31:0] RegP2;
		reg [31:0] RegP3;
		reg [31:0] RegP4;
		reg [31:0] RegP5;
		reg [31:0] RegP6;
		reg [31:0] RegP7;

		reg [31:0] next_RegP0;
		reg [31:0] next_RegP1;
		reg [31:0] next_RegP2;
		reg [31:0] next_RegP3;
		reg [31:0] next_RegP4;
		reg [31:0] next_RegP5;
		reg [31:0] next_RegP6;
		reg [31:0] next_RegP7;

	reg [NUM_WEIGHTS-1:0][31:0] Reg_R;
		reg [31:0] RegR0;
		reg [31:0] RegR1;
		reg [31:0] RegR2;
		reg [31:0] RegR3;
		reg [31:0] RegR4;
		reg [31:0] RegR5;
		reg [31:0] RegR6;
		reg [31:0] RegR7;

		reg [31:0] next_RegR0;
		reg [31:0] next_RegR1;
		reg [31:0] next_RegR2;
		reg [31:0] next_RegR3;
		reg [31:0] next_RegR4;
		reg [31:0] next_RegR5;
		reg [31:0] next_RegR6;
		reg [31:0] next_RegR7;

		reg [31:0] RegR0p;
		reg [31:0] RegR1p;
		reg [31:0] RegR2p;
		reg [31:0] RegR3p;
		reg [31:0] RegR4p;
		reg [31:0] RegR5p;
		reg [31:0] RegR6p;
		reg [31:0] RegR7p;

		reg [31:0] RegR0f;
		reg [31:0] RegR1f;
		reg [31:0] RegR2f;
		reg [31:0] RegR3f;
		reg [31:0] RegR4f;
		reg [31:0] RegR5f;
		reg [31:0] RegR6f;
		reg [31:0] RegR7f;

	reg [NUM_PIXELS-1:0][31:0] Reg_F;
	reg [31:0] RegF;
	reg [31:0] next_RegF;

	reg [NUM_PIXELS-1:0][31:0] Reg_M;
	reg [31:0] RegM;
	reg [31:0] next_RegM;

	reg [31:0] RegA;
	reg [15:0] iterator_row;
	reg [08:0] iterator_wei;

	reg clr;
	reg set_results;

	FP_AddSub FPSub(
		//input
		.aclr (clr),
		.add_sub (0),
		.clock (clk),
		.dataa (RegF),
		.datab (RegM),
		//output
		.result (RegA)
	);

	//MULTs-----------------------
		FP_Mult FPMult0(
			//input
			.aclr (clr),
			.clock (clk),
			.dataa (RegP0),
			.datab (RegA),
			//output
			.result (RegR0p)
		);
	
		FP_Mult FPMult1(
			//input
			.aclr (clr),
			.clock (clk),
			.dataa (RegP1),
			.datab (RegA),
			//output
			.result (RegR1p)
		);

		FP_Mult FPMult2(
			//input
			.aclr (clr),
			.clock (clk),
			.dataa (RegP2),
			.datab (RegA),
			//output
			.result (RegR2p)
		);

		FP_Mult FPMult3(
			//input
			.aclr (clr),
			.clock (clk),
			.dataa (RegP3),
			.datab (RegA),
			//output
			.result (RegR3p)
		);

		FP_Mult FPMult4(
			//input
			.aclr (clr),
			.clock (clk),
			.dataa (RegP4),
			.datab (RegA),
			//output
			.result (RegR4p)
		);

		FP_Mult FPMult5(
			//input
			.aclr (clr),
			.clock (clk),
			.dataa (RegP5),
			.datab (RegA),
			//output
			.result (RegR5p)
		);

		FP_Mult FPMult6(
			//input
			.aclr (clr),
			.clock (clk),
			.dataa (RegP6),
			.datab (RegA),
			//output
			.result (RegR6p)
		);

		FP_Mult FPMult7(
			//input
			.aclr (clr),
			.clock (clk),
			.dataa (RegP7),
			.datab (RegA),
			//output
			.result (RegR7p)
		);
	//----------------------------

	//ADDs------------------------
		FP_AddSub FPAdd0(
			//input
			.aclr (clr),
			.add_sub (1),
			.clock (clk),
			.dataa (RegR0),
			.datab (RegR0p),
			//output
			.result (RegR0f)
		);

		FP_AddSub FPAdd1(
			//input
			.aclr (clr),
			.add_sub (1),
			.clock (clk),
			.dataa (RegR1),
			.datab (RegR1p),
			//output
			.result (RegR1f)
		);

		FP_AddSub FPAdd2(
			//input
			.aclr (clr),
			.add_sub (1),
			.clock (clk),
			.dataa (RegR2),
			.datab (RegR2p),
			//output
			.result (RegR2f)
		);

		FP_AddSub FPAdd3(
			//input
			.aclr (clr),
			.add_sub (1),
			.clock (clk),
			.dataa (RegR3),
			.datab (RegR3p),
			//output
			.result (RegR3f)
		);

		FP_AddSub FPAdd4(
			//input
			.aclr (clr),
			.add_sub (1),
			.clock (clk),
			.dataa (RegR4),
			.datab (RegR4p),
			//output
			.result (RegR4f)
		);

		FP_AddSub FPAdd5(
			//input
			.aclr (clr),
			.add_sub (1),
			.clock (clk),
			.dataa (RegR5),
			.datab (RegR5p),
			//output
			.result (RegR5f)
		);

		FP_AddSub FPAdd6(
			//input
			.aclr (clr),
			.add_sub (1),
			.clock (clk),
			.dataa (RegR6),
			.datab (RegR6p),
			//output
			.result (RegR6f)
		);

		FP_AddSub FPAdd7(
			//input
			.aclr (clr),
			.add_sub (1),
			.clock (clk),
			.dataa (RegR7),
			.datab (RegR7p),
			//output
			.result (RegR7f)
		);
	//----------------------------



typedef enum{
				IDLE,

				SUB0,
				SUB1,
				SUB2,
				SUB3,
				SUB4,
				SUB5,
				SUB6,

				MULT0,
				MULT1,
				MULT2,
				MULT3,
				MULT4,

				ADD0,
				ADD1,
				ADD2,
				ADD3,
				ADD4,
				ADD5,
				ADD6,

				ASSIGN_RESULT
			} state_t;

state_t state, nextState;

	assign data_out = Reg_R;
	assign Reg_P = p_in;
	assign Reg_F = f_in;
	assign Reg_M = m_in;
	assign iterator_row = pixel_iter;
	assign iterator_wei = weight_iter;

	always_ff @(posedge clk or negedge rst_n) begin
		if(~rst_n) begin
			Reg_R <= '{default: '0 };
			state <= IDLE;
			clr <= 1;

			RegF <= '{default: '0 };
			RegM <= '{default: '0 };

			RegP0 <= '{default: '0 };
			RegP1 <= '{default: '0 };
			RegP2 <= '{default: '0 };
			RegP3 <= '{default: '0 };
			RegP4 <= '{default: '0 };
			RegP5 <= '{default: '0 };
			RegP6 <= '{default: '0 };
			RegP7 <= '{default: '0 };

			RegR0 <= '{default: '0 };
			RegR1 <= '{default: '0 };
			RegR2 <= '{default: '0 };
			RegR3 <= '{default: '0 };
			RegR4 <= '{default: '0 };
			RegR5 <= '{default: '0 };
			RegR6 <= '{default: '0 };
			RegR7 <= '{default: '0 };

		end else if (clear) begin
			Reg_R <= '{default: '0 };
			clr <= 1;

			RegF <= '{default: '0 };
			RegM <= '{default: '0 };

			RegP0 <= '{default: '0 };
			RegP1 <= '{default: '0 };
			RegP2 <= '{default: '0 };
			RegP3 <= '{default: '0 };
			RegP4 <= '{default: '0 };
			RegP5 <= '{default: '0 };
			RegP6 <= '{default: '0 };
			RegP7 <= '{default: '0 };

			RegR0 <= '{default: '0 };
			RegR1 <= '{default: '0 };
			RegR2 <= '{default: '0 };
			RegR3 <= '{default: '0 };
			RegR4 <= '{default: '0 };
			RegR5 <= '{default: '0 };
			RegR6 <= '{default: '0 };
			RegR7 <= '{default: '0 };

		end else begin
			state <= nextState;
			clr <= 0;

			RegF <= next_RegF;
			RegM <= next_RegM;

			RegP0 <= next_RegP0;
			RegP1 <= next_RegP1;
			RegP2 <= next_RegP2;
			RegP3 <= next_RegP3;
			RegP4 <= next_RegP4;
			RegP5 <= next_RegP5;
			RegP6 <= next_RegP6;
			RegP7 <= next_RegP7;

			RegR0 <= next_RegR0;
			RegR1 <= next_RegR1;
			RegR2 <= next_RegR2;
			RegR3 <= next_RegR3;
			RegR4 <= next_RegR4;
			RegR5 <= next_RegR5;
			RegR6 <= next_RegR6;
			RegR7 <= next_RegR7;

			if(set_results) begin
				Reg_R[iterator_wei] <= RegR0f;
				Reg_R[iterator_wei+1] <= RegR1f;
				Reg_R[iterator_wei+2] <= RegR2f;
				Reg_R[iterator_wei+3] <= RegR3f;
				Reg_R[iterator_wei+4] <= RegR4f;
				Reg_R[iterator_wei+5] <= RegR5f;
				Reg_R[iterator_wei+6] <= RegR6f;
				Reg_R[iterator_wei+7] <= RegR7f;
			end
		end
	end

	always_comb begin

		next_RegF = Reg_F[iterator_row];
		next_RegM = Reg_M[iterator_row];

		next_RegP0 = Reg_P[0][iterator_row];
		next_RegP1 = Reg_P[1][iterator_row];
		next_RegP2 = Reg_P[2][iterator_row];
		next_RegP3 = Reg_P[3][iterator_row];
		next_RegP4 = Reg_P[4][iterator_row];
		next_RegP5 = Reg_P[5][iterator_row];
		next_RegP6 = Reg_P[6][iterator_row];
		next_RegP7 = Reg_P[7][iterator_row];

		next_RegR0 = Reg_R[iterator_wei];
		next_RegR1 = Reg_R[iterator_wei+1];
		next_RegR2 = Reg_R[iterator_wei+2];
		next_RegR3 = Reg_R[iterator_wei+3];
		next_RegR4 = Reg_R[iterator_wei+4];
		next_RegR5 = Reg_R[iterator_wei+5];
		next_RegR6 = Reg_R[iterator_wei+6];
		next_RegR7 = Reg_R[iterator_wei+7];

		nextState = state;
		set_results = 0;

		case (state)
			IDLE: begin
				if(enable) begin
					nextState = SUB0;
				end
			end

			SUB0: begin
				nextState = SUB1;
			end
			SUB1: begin
				nextState = SUB2;
			end
			SUB2: begin
				nextState = SUB3;
			end
			SUB3: begin
				nextState = SUB4;
			end
			SUB4: begin
				nextState = SUB5;
			end
			SUB5: begin
				nextState = SUB6;
			end
			SUB6: begin
				nextState = MULT0;
			end

			MULT0: begin
				nextState = MULT1;
			end
			MULT1: begin
				nextState = MULT2;
			end
			MULT2: begin
				nextState = MULT3;
			end
			MULT3: begin
				nextState = MULT4;
			end
			MULT4: begin
				nextState = ADD0;
			end

			ADD0: begin
				nextState = ADD1;
			end
			ADD1: begin
				nextState = ADD2;
			end
			ADD2: begin
				nextState = ADD3;
			end
			ADD3: begin
				nextState = ADD4;
			end
			ADD4: begin
				nextState = ADD5;
			end
			ADD5: begin
				nextState = ADD6;
			end
			ADD6: begin
				nextState = ASSIGN_RESULT;
			end

			ASSIGN_RESULT: begin
				set_results = 1;
				nextState = IDLE;
			end
		endcase
	end

endmodule // ProjReg