

`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    16:41:04 11/28/2023 
// Design Name: 
// Module Name:    Verilog4 
// Project Name: 
// Target Devices: 
// Tool versions: 
// Description: 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////


/*

Make sure to run fitter like in command line, it allows you to change -optimize to "density" which makes things fit!

cd ~/Xilinx/JustPIC ; ~/Xilinx/14.7/ISE_DS/ISE/bin/lin64/cpldfit -intstyle ise -p xc9572xl-5-VQ44 -ofmt vhdl -optimize density -htmlrpt -loc on -slew fast -init low -inputs 54 -pterms 25 -unused float -power std -terminate keeper Verilog3.ngd ; ~/Xilinx/14.7/ISE_DS/ISE/bin/lin64/hprep6 -s IEEE1149 -n Verilog3 -i Verilog3

cd /usr/share ; sudo xc3sprog -c xpc ~/Xilinx/JustPIC/Verilog3.jed

Use 'xc3sprog' to program the .jed file into the CPLD, but you need to follow some instructions:

Notes for the USB cable
=======================

To use the device as an ordinary user, put the following line in a new
file "libusb-driver.rules" in /etc/udev/rules.d/ and restart udev:
ACTION=="add", SUBSYSTEMS=="usb", ATTRS{idVendor}=="03fd", MODE="666"


If your cable does not have the ID 03fd:0008 in the output of lsusb,
the initial firmware has not been loaded (loading it changes the
product-ID from another value to 8). To load the firmware follow
these steps:

1. Run ./setup_pcusb in this directory, this should set up everything
   correctly:
   - When $XILINX is set correctly:
     $ ./setup_pcusb
   - When $XILINX is not set, and ISE is installed in /opt/Xilinx/13.1:
     $ ./setup_pcusb /opt/Xilinx/13.1/ISE_DS/ISE

Old instructions, use only when the above script did not work:

1. If you have no /etc/udev/rules.d/xusbdfwu.rules file, copy it from
   /path/to/ISE/bin/lin/xusbdfwu.rules to /etc/udev/rules.d/xusbdfwu.rules

2. If you are running a newer version of udev (as in Debian Squeeze and
   Ubuntu 9.10), you need to adapt the rules-file to the new udev-version:
   sed -i -e 's/TEMPNODE/tempnode/' -e 's/SYSFS/ATTRS/g' -e 's/BUS/SUBSYSTEMS/' /etc/udev/rules.d/xusbdfwu.rules

3. Install the package containing /sbin/fxload from your linux distribution.
   It is usually called "fxload"

4. copy the files /path/to/ISE/bin/lin/xusb*.hex to /usr/share/

5. restart udev and re-plug the cable with 'service udev restart'

That should do it???

*/

// This is for the PIC-only system.  
// It should run at 6.28 MHz, giving 16 colors at 320x240 and using 32KB of RAM.
// The PIC will interface using it's Parallel Master Port (technically Parallel Slave Port here)
// much like a 6502 would.  The PIC takes care of it's own addresses.
// This CPLD should have enough room to do vertical OR horizontal scrolling if pre-programmed to do so.


module Verilog3(
	input master_clock, // 25.175 MHz
	inout video, // 6.28 MHz
	output not_video, // inverted clock
	input write, // signals from PIC, want to write when low
	input addr, // signals from PIC, unused?
	output reg hsync, // video sync signals
	output reg vsync,
	output ram_ce, // used to be /CE for RAM, pulled down, now is another clock
	output ram_oe, // /OE for RAM
	output ram_we, // /WE for RAM
	output pic_prd, // parallel read (high enabled)
	output pic_pwr, // parallel write (high enabled)
	output pic_pcs, // use to be parallel chip select (high enabled), now is another clock
	inout [7:0] address,
	inout [7:0] data,
	output latch, // latches upper address (during hsync)
	output reg red,
	output reg green,
	output reg blue,
	output reg intensity
    );

reg half;
reg quarter;
reg eighth;
reg [17:0] video_addr; // video addresses
reg [5:0] color_data;
reg hblank; // blanking values
reg vblank;
reg vscreen;
reg prev;
reg ready_write;
reg ready_read;
reg ready_video;
reg [7:0] preset_colors;

assign video = 1'bz;
assign not_video = master_clock;
assign ram_ce = ~eighth;
assign pic_pcs = eighth;

assign latch = ~hsync;

// these are A0-A7, changed each pixel, inactive during phi2 high or booting,
// these supply A8-A15 during h-sync for external latch
assign address[6:0] = ((~latch && ~eighth) ? video_addr[16:10] :
	((~video) ? video_addr[6:0] :
	7'bzzzzzzz));
assign address[7] = ((~latch && ~eighth) ? video_addr[17] :
	((~eighth) ? prev :
	1'bz));
	
//assign ram_ce = 1'b0;
assign ram_oe = (~eighth ? 1'b0 : (~ready_read ? 1'b0 : 1'b1));
assign ram_we = (~eighth ? 1'b1 : ((~ready_write && quarter) ? 1'b0 : 1'b1));

//assign pic_pcs = (~eighth ? 1'b1 : 1'b0);
assign pic_prd = (~eighth ? 1'b1 : (~ready_write ? 1'b0 : 1'b1));
assign pic_pwr = (~eighth ? 1'b1 : ((~ready_read && quarter) ? 1'b0 : 1'b1)); //((quarter && ~half) || (~quarter && half)) ? 1'b0 : 1'b1)));

assign data[7:0] = 8'bzzzzzzzz;

always @(negedge master_clock) begin
	
	ready_video <= video;
	
	if (ready_video && ~video) begin
		half <= 1'b0;
		quarter <= 1'b0;
		eighth <= ~eighth;
	end
	else if (~ready_video && ~video) begin
		half <= 1'b1;
		quarter <= 1'b0;
	end
	else if (~ready_video && video) begin
		half <= 1'b0;
		quarter <= 1'b1;
	end
	else if (ready_video && video) begin
		half <= 1'b1;
		quarter <= 1'b1;
	end
	
	if (half && ~quarter && eighth) begin
		if (hblank && vscreen && ~vblank) begin
			red <= 1'b0;
			green <= 1'b0;
			blue <= 1'b0;
			intensity <= 1'b1;
		end
		else if (hblank && vscreen && vblank) begin
			if (preset_colors[7:0] != 8'b00000000) begin
				red <= (color_data[5] & preset_colors[7]) | (color_data[4] & preset_colors[3]);
				green <= (color_data[5] & preset_colors[6]) | (color_data[4] & preset_colors[2]);
				blue <= (color_data[5] & preset_colors[5]) | (color_data[4] & preset_colors[1]);
				intensity <= (color_data[5] & preset_colors[4]) | (color_data[4] & preset_colors[0]);
			end	
		end
		else begin
			red <= 1'b0;
			green <= 1'b0;
			blue <= 1'b0;
			intensity <= 1'b0;
		end
	end
	else if (half && quarter && eighth) begin	
		ready_write <= 1'b1;
		ready_read <= 1'b1;
	
		if (hblank && vscreen && ~vblank) begin
			red <= 1'b0;
			green <= 1'b0;
			blue <= 1'b0;
			intensity <= 1'b1;
		end
		else if (hblank && vscreen && vblank) begin
			if (preset_colors[7:0] == 8'b00000000) begin
				red <= color_data[3];
				green <= color_data[2];
				blue <= color_data[1];
				intensity <= color_data[0];
			end
			else begin
				red <= (color_data[3] & preset_colors[7]) | (color_data[2] & preset_colors[3]);
				green <= (color_data[3] & preset_colors[6]) | (color_data[2] & preset_colors[2]);
				blue <= (color_data[3] & preset_colors[5]) | (color_data[2] & preset_colors[1]);
				intensity <= (color_data[3] & preset_colors[4]) | (color_data[2] & preset_colors[0]);
			end
		end
		else begin
			red <= 1'b0;
			green <= 1'b0;
			blue <= 1'b0;
			intensity <= 1'b0;
		end
	end
	else if (half && ~quarter && ~eighth) begin	
		if (hblank && vscreen && ~vblank) begin
			red <= 1'b0;
			green <= 1'b0;
			blue <= 1'b0;
			intensity <= 1'b1;
		end
		else if (hblank && vscreen && vblank) begin
			if (preset_colors[7:0] != 8'b00000000) begin
				red <= (color_data[1] & preset_colors[7]) | (color_data[0] & preset_colors[3]);
				green <= (color_data[1] & preset_colors[6]) | (color_data[0] & preset_colors[2]);
				blue <= (color_data[1] & preset_colors[5]) | (color_data[0] & preset_colors[1]);
				intensity <= (color_data[1] & preset_colors[4]) | (color_data[0] & preset_colors[0]);
			end
		end
		else begin
			red <= 1'b0;
			green <= 1'b0;
			blue <= 1'b0;
			intensity <= 1'b0;
		end
	end
	else if (half && quarter && ~eighth) begin
		color_data[5:0] <= data[5:0];
		
		if (addr) begin
			ready_write <= write;
			ready_read <= 1'b1;
		end
		else begin
			ready_write <= 1'b1;
			ready_read <= write;
		end
	
		if (hblank && vscreen && ~vblank) begin
			red <= 1'b0;
			green <= 1'b0;
			blue <= 1'b0;
			intensity <= 1'b1;
		end
		else if (hblank && vscreen && vblank) begin
			if (preset_colors[7:0] == 8'b00000000) begin
				red <= data[7];
				green <= data[6];
				blue <= data[5];
				intensity <= data[4];
			end
			else begin
				red <= (data[7] & preset_colors[7]) | (data[6] & preset_colors[3]);
				green <= (data[7] & preset_colors[6]) | (data[6] & preset_colors[2]);
				blue <= (data[7] & preset_colors[5]) | (data[6] & preset_colors[1]);
				intensity <= (data[7] & preset_colors[4]) | (data[6] & preset_colors[0]);
			end
		end
		else begin
			red <= 1'b0;
			green <= 1'b0;
			blue <= 1'b0;
			intensity <= 1'b0;
		end
	end
	
	if (half && quarter && ~eighth) begin
	
		// video sync signals
		
		if (video_addr[7:0] == 8'b01101000) begin
			hsync <= 1'b1;
			
			if (vblank) begin
				preset_colors[7:0] <= data[7:0];
			end
		end
		
		if (video_addr[7:0] == 8'b01111000) begin
			hsync <= 1'b0;
			prev <= video_addr[9];
		end

		if (video_addr[7:0] == 8'b10000011) begin	
			if (video_addr[17:8] == 10'b1000101101) begin //10'b1001011001) begin
				vsync <= 1'b1;
			end
			
			if (video_addr[17:8] == 10'b1000110001) begin //10'b1001011101) begin
				vsync <= 1'b0;
			end

			if (video_addr[17:8] == 10'b1001110011) begin
				video_addr[17:8] <= 10'b0000000000;
			end
			else begin
				video_addr[17:8] <= video_addr[17:8] + 1;
			end
			
			video_addr[7:0] <= 8'b00000000;
		end
		else begin
			video_addr[7:0] <= video_addr[7:0] + 1;
		end
	end
	
	if (half && ~quarter && ~eighth) begin
	
		// video blanking signals
	
		if (video_addr[7:0] == 8'b00000000) begin
			hblank <= 1'b1;
		end
		
		if (video_addr[7:0] == 8'b01100100) begin
			hblank <= 1'b0;
		end
		
		if (video_addr[7:0] == 8'b10000011) begin	
			if (video_addr[17:8] == 10'b0111111111) begin
				vblank <= 1'b0;
			end
			
			if (video_addr[17:8] == 10'b1000101100) begin //10'b1001011000) begin
				vscreen <= 1'b0;
			end
			
			if (video_addr[17:8] == 10'b1001001000) begin //10'b0000000000) begin
				vscreen <= 1'b1;
			end
			
			if (video_addr[17:8] == 10'b0000000000) begin
				vblank <= 1'b1;
			end
		end
	end
end

	
endmodule




