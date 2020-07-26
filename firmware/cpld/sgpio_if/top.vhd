--
-- Copyright 2012 Jared Boone
-- Copyright 2013 Benjamin Vernoux
--
-- This file is part of HackRF.
--
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 2, or (at your option)
-- any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program; see the file COPYING.  If not, write to
-- the Free Software Foundation, Inc., 51 Franklin Street,
-- Boston, MA 02110-1301, USA.

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.std_logic_unsigned.all;

library UNISIM;
use UNISIM.vcomponents.all;

entity top is
    Port(
        HOST_DATA       : inout std_logic_vector(7 downto 0);
        HOST_CAPTURE    : out   std_logic;
		  HOST_SYNC_EN    : in    std_logic;
        HOST_SYNC_CMD   : out   std_logic;
        HOST_SYNC       : in    std_logic;
        HOST_DISABLE    : in    std_logic;
        HOST_DIRECTION  : in    std_logic;
        HOST_Q_INVERT   : in    std_logic;

        DA              : in    std_logic_vector(7 downto 0);
        DD              : out   std_logic_vector(9 downto 0);

        CODEC_CLK       : in    std_logic;
        CODEC_X2_CLK    : in    std_logic
    );

end top;

architecture Behavioral of top is
    signal codec_clk_i : std_logic;
    signal adc_data_i : std_logic_vector(7 downto 0);
    signal dac_data_o : std_logic_vector(9 downto 0);

    signal host_clk_i : std_logic;

    type transfer_direction is (from_adc, to_dac);
    signal transfer_direction_i : transfer_direction;

    signal host_data_enable_i : std_logic;
    signal host_data_capture_o : std_logic;
	 signal host_sync_enable : std_logic := '0';
    signal host_sync_o : std_logic := '0';
    signal host_sync_i : std_logic := '0';
    signal host_sync_latched : std_logic := '0';

    signal data_from_host_i : std_logic_vector(7 downto 0);
    signal data_to_host_o : std_logic_vector(7 downto 0);

    signal q_invert : std_logic;
    signal rx_q_invert_mask : std_logic_vector(7 downto 0);
    signal tx_q_invert_mask : std_logic_vector(7 downto 0);

begin
    
    ------------------------------------------------
    -- Codec interface
    
    adc_data_i <= DA(7 downto 0);    
    DD(9 downto 0) <= dac_data_o;
    
    ------------------------------------------------
    -- Clocks
    
    codec_clk_i <= CODEC_CLK;
    
    BUFG_host : BUFG
    port map (
        O => host_clk_i,
        I => CODEC_X2_CLK
    );

    ------------------------------------------------
    -- SGPIO interface
    
    HOST_DATA <= data_to_host_o when transfer_direction_i = from_adc
                                else (others => 'Z');
    data_from_host_i <= HOST_DATA;

    HOST_CAPTURE <= host_data_capture_o;
	 host_sync_enable <= HOST_SYNC_EN;
	 host_sync_i <= HOST_SYNC;
	 HOST_SYNC_CMD <= host_sync_o;
	 
    host_data_enable_i <= not HOST_DISABLE;
    transfer_direction_i <= to_dac when HOST_DIRECTION = '1'
                                   else from_adc;
     
    ------------------------------------------------
        
    q_invert <= HOST_Q_INVERT;
    rx_q_invert_mask <= X"80" when q_invert = '1' else X"7f";
    tx_q_invert_mask <= X"7F" when q_invert = '1' else X"80";
     
    process(host_clk_i)
    begin
        if rising_edge(host_clk_i) then
            if codec_clk_i = '1' then
                -- I: non-inverted between MAX2837 and MAX5864
                data_to_host_o <= adc_data_i xor X"80";
            else
                -- Q: inverted between MAX2837 and MAX5864
                data_to_host_o <= adc_data_i xor rx_q_invert_mask;
            end if;
        end if;
    end process;
    
    process(host_clk_i)
    begin
        if rising_edge(host_clk_i) then
            if transfer_direction_i = to_dac then
                if codec_clk_i = '1' then
                    dac_data_o <= (data_from_host_i xor tx_q_invert_mask) & tx_q_invert_mask(0) & tx_q_invert_mask(0);
                else
                    dac_data_o <= (data_from_host_i xor X"80") & "00";
                end if;
            else
                dac_data_o <= (dac_data_o'high => '0', others => '1');
            end if;
        end if;
    end process;
    
    process (host_data_enable_i, host_sync_i)
    begin
        host_sync_o <= host_data_enable_i;
        if host_data_enable_i = '1' then
            if rising_edge(host_sync_i) then
                host_sync_latched <= host_sync_i;
            end if;
        else
            host_sync_latched <= '0';
        end if;
    end process;
    
    process(host_clk_i)
    begin
        if rising_edge(host_clk_i) then
            if transfer_direction_i = to_dac then
                if codec_clk_i = '1' then
                    host_data_capture_o <= host_data_enable_i and (host_sync_latched or not host_sync_enable);
                end if;
            else
                if codec_clk_i = '0' then
                    host_data_capture_o <= host_data_enable_i and (host_sync_latched or not host_sync_enable);
                end if; 
            end if;
        end if;
    end process;
    
end Behavioral;
