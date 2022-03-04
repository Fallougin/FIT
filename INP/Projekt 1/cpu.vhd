-- cpu.vhd: Simple 8-bit CPU (BrainLove interpreter)
-- Copyright (C) 2021 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): xsuvor00
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

-- ----------------------------------------------------------------------------
--                        Entity declaration
-- ----------------------------------------------------------------------------
entity cpu is
 port (
   CLK   : in std_logic;  -- hodinovy signal
   RESET : in std_logic;  -- asynchronni reset procesoru
   EN    : in std_logic;  -- povoleni cinnosti procesoru
 
   -- synchronni pamet ROM
   CODE_ADDR : out std_logic_vector(11 downto 0); -- adresa do pameti
   CODE_DATA : in std_logic_vector(7 downto 0);   -- CODE_DATA <- rom[CODE_ADDR] pokud CODE_EN='1'
   CODE_EN   : out std_logic;                     -- povoleni cinnosti
   
   -- synchronni pamet RAM
   DATA_ADDR  : out std_logic_vector(9 downto 0); -- adresa do pameti
   DATA_WDATA : out std_logic_vector(7 downto 0); -- ram[DATA_ADDR] <- DATA_WDATA pokud DATA_EN='1'
   DATA_RDATA : in std_logic_vector(7 downto 0);  -- DATA_RDATA <- ram[DATA_ADDR] pokud DATA_EN='1'
   DATA_WREN  : out std_logic;                    -- cteni z pameti (DATA_WREN='0') / zapis do pameti (DATA_WREN='1')
   DATA_EN    : out std_logic;                    -- povoleni cinnosti
   
   -- vstupni port
   IN_DATA   : in std_logic_vector(7 downto 0);   -- IN_DATA obsahuje stisknuty znak klavesnice pokud IN_VLD='1' a IN_REQ='1'
   IN_VLD    : in std_logic;                      -- data platna pokud IN_VLD='1'
   IN_REQ    : out std_logic;                     -- pozadavek na vstup dat z klavesnice
   
   -- vystupni port
   OUT_DATA : out  std_logic_vector(7 downto 0);  -- zapisovana data
   OUT_BUSY : in std_logic;                       -- pokud OUT_BUSY='1', LCD je zaneprazdnen, nelze zapisovat,  OUT_WREN musi byt '0'
   OUT_WREN : out std_logic                       -- LCD <- OUT_DATA pokud OUT_WE='1' a OUT_BUSY='0'
 );
end cpu;


-- ----------------------------------------------------------------------------
--                      Architecture declaration
-- ----------------------------------------------------------------------------
architecture behavioral of cpu is
-- PC
	signal pc_reg : std_logic_vector (11 downto 0);
	signal pc_inc : std_logic;
	signal pc_dec : std_logic;
	
-- POINTER
	signal ptr_reg : std_logic_vector (9 downto 0);
	signal ptr_inc : std_logic;
	signal ptr_dec : std_logic;
	
--CNT
	signal cnt_reg : std_logic_vector (11 downto 0);
	signal cnt_inc : std_logic;
	signal cnt_dec : std_logic;
	signal cnt_reset : std_logic;
	
--MUX
	signal mx_select : std_logic_vector (1 downto 0) := "00";
	signal mx_output : std_logic_vector (7 downto 0); 

-- STATES
	type fsm_state is (
		s_start,
		s_fetch,
		s_decode,

		s_ptr_inc,
		s_ptr_dec,

		s_inc_1, s_inc_2, s_inc_end,
		s_dec_1, s_dec_2, s_dec_end,
		
		s_while_0, s_while_1, s_while_2, s_while_3,
		s_while_end_0, s_while_end_1,s_while_end_2, s_while_end_3, s_while_end_4,
		
		s_write, s_write_done,

		s_get, s_get_done,
		
		s_break_start, s_break_end , s_break_enable,
		s_null
	);
	signal state : fsm_state := s_start;
	signal nState : fsm_state;

begin

--PC
	pc: process (CLK, RESET, pc_inc, pc_dec) 
	begin
		if RESET = '1' then
			pc_reg <= (others => '0');
		elsif rising_edge(CLK) then
			if pc_inc = '1' then
				pc_reg <= pc_reg + 1;
			elsif pc_dec = '1' then
				pc_reg <= pc_reg - 1;
			end if;	
		end if;	
	end process;
	CODE_ADDR <= pc_reg;
	
--PTR
	ptr: process (CLK, RESET, ptr_inc, ptr_dec) 
	begin
		if RESET = '1' then
			ptr_reg <= (others => '0');
		elsif rising_edge(CLK) then
			if ptr_inc = '1' then
				ptr_reg <= ptr_reg + 1;
			elsif ptr_dec = '1' then
				ptr_reg <= ptr_reg - 1;	
			end if;
		end if;	
	end process;	
	DATA_ADDR <= ptr_reg;

--CNT
	cnt: process(CLK,RESET, cnt_inc, cnt_dec)
	begin
		if RESET = '1' then
			cnt_reg <= (others => '0');
		elsif rising_edge(CLK) then
			if cnt_inc = '1' then
				cnt_reg <= cnt_reg + 1;
			elsif cnt_dec = '1' then
				cnt_reg <= cnt_reg - 1;
				
			end if;	
		end if;	
	end process;	
	OUT_DATA <= DATA_RDATA;

--MUX
	mux: process (CLK, RESET, mx_select) 
	begin
		if RESET = '1' then
			mx_output <= (others => '0');
		elsif rising_edge(CLK) then
   			case mx_select is
				when "00" => mx_output <= IN_DATA;
				when "01" => mx_output <= DATA_RDATA + 1;
				when "10" => mx_output <= DATA_RDATA - 1;
				when others =>
					mx_output <= (others => '0');
			end case;
		end if;
	end process;
	DATA_WDATA <= mx_output;
	
--FSM
	state_logic: process (CLK, RESET, EN) is
	begin
		if RESET = '1' then
			state <= s_start;
		elsif rising_edge(CLK) then
			if EN = '1' then
				state <= nState;
			end if;
		end if;
	end process;
	
	fsm: process (state, OUT_BUSY, IN_VLD, CODE_DATA, DATA_RDATA) is
	begin
		--initialization
		pc_inc <= '0';
		pc_dec <= '0';
		
		ptr_inc <= '0';
		ptr_dec <= '0';
		
		cnt_inc <= '0';
		cnt_dec <= '0';
		cnt_reset <= '0';
		
		CODE_EN <= '0';
		DATA_EN <= '0';
		DATA_WREN <= '0';
		IN_REQ <= '0';
		OUT_WREN <= '0';

		mx_select <= "00";

		case state is
		
			when s_start =>
				cnt_reset <= '1';
				nState <= s_fetch;
				
			when s_fetch =>
				CODE_EN <= '1';
				nState <= s_decode;
				
			when s_decode =>
				case CODE_DATA is
				
					when X"3E" => nState <= s_ptr_inc;
					when X"3C" => nState <= s_ptr_dec;

					when X"2B" => nState <= s_inc_1;
					when X"2D" => nState <= s_dec_1;

					when X"5B" => nState <= s_while_0;
					when X"5D" => nState <= s_while_end_0;

					when X"2E" => nState <= s_write;
					when X"2C" => nState <= s_get;

					when X"7E" => nState <= s_break_start;
					when X"00" => nState <= s_null;

					when others =>
						pc_inc <= '1';
						nState <= s_fetch;
	
				end case;

			when s_ptr_inc =>  		--0x3E -- >  
				pc_inc <= '1';
				ptr_inc <= '1';
				nState <= s_fetch;
				
			when s_ptr_dec =>		--0x3C  -- <
				pc_inc <= '1';
				ptr_dec <= '1';
				nState <= s_fetch;
			
			when s_inc_1 =>		--0x2B -- +
				DATA_EN <= '1';
				DATA_WREN <= '0';
				nState <= s_inc_2;
			when s_inc_2 =>
				mx_select <= "01";
				nState <= s_inc_end;
			when s_inc_end =>
				DATA_EN <= '1';
				DATA_WREN <= '1';
				pc_inc <= '1';
				nState <= s_fetch;

			when s_dec_1 =>		--0x2D -- -
				DATA_EN <= '1';
				DATA_WREN <= '0';
				nState <= s_dec_2;
			when s_dec_2 =>
				mx_select <= "10";
				nState <= s_dec_end;
			when s_dec_end =>
				DATA_EN <= '1';
				DATA_WREN <= '1';
				pc_inc <= '1';
				nState <= s_fetch;
				
			when s_write =>			--0x2E
				DATA_EN <= '1';
				DATA_WREN <= '0';
				nState <= s_write_done;
			when s_write_done =>
				if OUT_BUSY = '1' then
					DATA_EN <= '1';
					DATA_WREN <= '0';
					nState <= s_write_done;
				else
					OUT_WREN <= '1';
					pc_inc <= '1';
					nState <= s_fetch;
				end if;

			when s_get =>			--0x2C
				IN_REQ <= '1';
				mx_select <="00";
				nState <= s_get_done;
			when s_get_done => 
				if IN_VLD /= '1' then 
					IN_REQ <= '1';
					mx_select <= "00";
					nState <= s_get_done;
				else
					DATA_EN <= '1';	
					DATA_WREN <= '1';
					pc_inc <= '1';
					nState <= s_fetch;
				end if;		
				
			when s_while_0 =>	--0x5B -- [ 
				pc_inc <= '1';
				DATA_EN <= '1';
				DATA_WREN <= '0';
				nState <= s_while_1;
			when s_while_1 =>
				if DATA_RDATA = "00000000" then 
					cnt_inc <= '1';
					CODE_EN <= '1';
					nState <= s_while_2;
				else
					nState <= s_fetch;
				end if;				
			when s_while_2 =>
	      	if cnt_reg = "000000000000" then
					nState <= s_fetch;
				else	
					if CODE_DATA = X"5B" then 
						cnt_inc <= '1';
					elsif CODE_DATA = X"5D" then 
						cnt_dec <= '1';
					end if;
					pc_inc <= '1';
					nState <= s_while_3;
				end if;		
			when s_while_3 =>
				CODE_EN <= '1';
				nState <= s_while_2;
				
			when s_while_end_0 =>	--0x5D -- ] 
				DATA_EN <= '1';
				DATA_WREN <= '0';
				nState <= s_while_end_1;
			when s_while_end_1 =>
				if DATA_RDATA = "00000000" then
					pc_inc <= '1';
					nState <= s_fetch;
				else
					cnt_inc <= '1';
					pc_dec <= '1';
					nState <= s_while_end_4;
				end if;				
			when s_while_end_2 =>
	      	if cnt_reg = "000000000000" then
					nState <= s_fetch;
				else
					if CODE_DATA = X"5D" then 
						cnt_inc <= '1';
					elsif CODE_DATA = X"5B" then 
						cnt_dec <= '1';
					end if;
					nState <= s_while_end_3;
				end if;
			when s_while_end_3 =>
	      	if cnt_reg = "000000000000" then
					pc_inc <='1';
				else
					pc_dec <='1';
				end if;
				nState <= s_while_end_4;
			when s_while_end_4 =>
				CODE_EN <= '1';
				nState <= s_while_end_2;	
			
			when s_break_start =>	--0x7E -- ~
				cnt_inc <= '1';
				pc_inc <= '1';
				nState <= s_break_enable;				
			when s_break_end =>
	      	if cnt_reg /= "000000000000" then
					if CODE_DATA = X"5B" then cnt_inc <= '1';
					elsif CODE_DATA = X"5D" then cnt_dec <= '1';
					end if;
					pc_inc <= '1';
					nState <= s_break_enable;
				else
					nState <= s_fetch;
				end if;		
			when s_break_enable =>
				CODE_EN <= '1';
				nState <= s_break_end;
			
			when s_null =>				--0x00
				nState <= s_null;
			
			when others =>
				null;
				
		end case;			
	end process;	
end behavioral;

