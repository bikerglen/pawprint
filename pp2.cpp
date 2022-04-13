//---------------------------------------------------------------------------------------------
// includes
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>


//---------------------------------------------------------------------------------------------
// defines
//

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))


//---------------------------------------------------------------------------------------------
// prototypes
//

void RenderEaglePawPad (FILE *fout, bool stopLayer, bool wire);
void CalculateSmdPadXY (int index, double *x, double *y);


//---------------------------------------------------------------------------------------------
// globals
//


//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// BEGIN TWEMOJI DERIVED ARTWORK STUFF
//
// paw print artwork derived from twemoji 1f43e.svg
// https://github.com/twitter/twemoji/blob/master/assets/svg/1f43e.svg
// Twemoji Graphics licensed under CC-BY 4.0: https://creativecommons.org/licenses/by/4.0/
//

// scale is value to scale artwork down to the final artwork size on the board
// xoffset and yoffset are the center of the artwork in the svg file in pixels
#define SCALE    0.265
#define XOFFSET 11.0
#define YOFFSET  8.9

// bezier control points for each pad, same units as svg file, i.e., pixels
// derived from original svg file using affinity photo to save a new file with
// absolute moveto and curveto (M,C) instead of relative versions (m,c)
double pads[5][13][2] = {
	{   // left small digit
		{  5.074,  8.037 },
		{  5.467,  9.372 }, {  5.081, 10.662 }, {  4.212, 10.918 },
		{  3.342, 11.174 }, {  2.319, 10.299 }, {  1.926,  8.963 },
		{  1.533,  7.628 }, {  1.918,  6.338 }, {  2.788,  6.082 },
		{  3.658,  5.826 }, {  4.681,  6.701 }, {  5.074,  8.037 }
	},
	{   // left big digit
		{  9.964,  4.122 },
		{ 10.330,  6.020 }, {  9.747,  7.728 }, {  8.662,  7.937 },
		{  7.578,  8.145 }, {  6.402,  6.776 }, {  6.037,  4.878 },
		{  5.670,  2.980 }, {  6.253,  1.272 }, {  7.338,  1.063 },
		{  8.423,  0.854 }, {  9.599,  2.224 }, {  9.964,  4.122 }
	},
	{	// right big digit
		{ 15.960,  4.900 },
		{ 15.573,  6.794 }, { 14.382,  8.150 }, { 13.300,  7.929 },
		{ 12.218,  7.708 }, { 11.654,  5.993 }, { 12.041,  4.099 },
		{ 12.428,  2.205 }, { 13.619,  0.849 }, { 14.701,  1.070 },
		{ 15.783,  1.292 }, { 16.346,  3.006 }, { 15.960,  4.900 }
	},
	{	// right small digit
		{ 20.074,  8.981 },
		{ 19.667, 10.313 }, { 18.632, 11.177 }, { 17.762, 10.911 },
		{ 16.892, 10.645 }, { 16.518,  9.350 }, { 16.925,  8.018 },
		{ 17.332,  6.686 }, { 18.367,  5.822 }, { 19.237,  6.088 },
		{ 20.106,  6.354 }, { 20.481,  7.649 }, { 20.074,  8.981 }
	},
	{   // "palm"
		{ 16.706, 16.113 }, 
		{ 16.706, 20.596 }, { 14.152, 18.151 }, { 11.000, 18.151 },
		{  7.849, 18.151 }, {  5.294, 20.597 }, {  5.294, 16.113 },
		{  5.294, 13.187 }, {  7.849, 10.000 }, { 11.000, 10.000 },
		{ 14.151, 10.000 }, { 16.706, 13.187 }, { 16.706, 16.113 }
	}
};

// pad center points and stop mask scale factors, used for expanding each pad outward when 
// creatiing stop mask, used affinity designer to figure centers out, checked by hand using 
// bezier control points above. stop mask is similar to overexposing solder layer and exact 
// values were determined experimentally
double padcenters[5][3] = {
	{  3.5,  8.5, 1.10 }, // left small, 
	{  8.0,  4.5, 1.10 }, // left big
	{ 14.0,  4.5, 1.10 }, // right small
	{ 18.5,  8.5, 1.10 }, // right big
	{ 11.0, 14.1, 1.05 }, // palm
};

//
// END DERIVED ARTWORK
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


//---------------------------------------------------------------------------------------------
// main
//

int main (int argc, char *argv[])
{
	FILE *fout;
	double x, y;

	fout = fopen ("pp2.scr", "w");

	// grid needs to be in mm
	fprintf (fout, "grid mm;\n");

	// wire bend needs to be at any angle
	fprintf (fout, "set wire_bend 2;\n");

	//----------------------------------------
	// target footprint
	//----------------------------------------

	// package name
	fprintf (fout, "edit 'PAWPAD-TARGET.pac'\n");

	// alignment holes, on equilateral triangle
	fprintf (fout, "hole 1 (-2.4537 -1.5);\n");
	fprintf (fout, "hole 1 (0 2.75);\n");
	fprintf (fout, "hole 1 (2.4537 -1.5);\n");

	// top solder artwork
	fprintf (fout, "change layer top\n");
	fprintf (fout, "change width 6mil\n");
	RenderEaglePawPad (fout, false, false);

	// top stop artwork
	fprintf (fout, "change layer tstop\n");
	fprintf (fout, "change width 6mil\n");
	RenderEaglePawPad (fout, true, false);

	// smd pads
	CalculateSmdPadXY (0, &x, &y);
	fprintf (fout, "smd 0.4x1 -0 R15  'P1' (%8.3f %8.3f);\n", x, y);
	CalculateSmdPadXY (1, &x, &y);
	fprintf (fout, "smd 0.4x1 -0 R5  'P2'  (%8.3f %8.3f);\n", x, y);
	CalculateSmdPadXY (2, &x, &y);
	fprintf (fout, "smd 0.4x1 -0 R-5 'P3'  (%8.3f %8.3f);\n", x, y);
	CalculateSmdPadXY (3, &x, &y);
	fprintf (fout, "smd 0.4x1 -0 R-15 'P4' (%8.3f %8.3f);\n", x, y);
	x = 0;
	y = -1.584;
	fprintf (fout, "smd 1x2 -0 R90   'P5'  (%8.3f %8.3f);\n", x, y);

	// name
	fprintf (fout, "change font vector\n");
	fprintf (fout, "change size 32mil\n");
	fprintf (fout, "change ratio 16%%\n");
	fprintf (fout, "change align top-center\n");
	fprintf (fout, "change layer tNames\n");
	fprintf (fout, "text '>NAME' (0 -3)\n");

	//----------------------------------------
	// test fixture footprint
	//----------------------------------------

	// package name
	fprintf (fout, "edit 'PAWPAD-FIXTURE.pac'\n");

	// top silkscreen artwork
	fprintf (fout, "change layer tPlace\n");
	fprintf (fout, "change width 4mil\n");
	RenderEaglePawPad (fout, false, true);

	// bottom silkscreen artwork
	fprintf (fout, "change layer bPlace\n");
	fprintf (fout, "change width 4mil\n");
	RenderEaglePawPad (fout, false, true);

	// alignment holes, on equilateral triangle
	fprintf (fout, "change drill 0.9mm\n");
	fprintf (fout, "pad auto round 'P6' (-2.4537 -1.5);\n");
	fprintf (fout, "pad auto round 'P7' (0 2.75);\n");
	fprintf (fout, "pad auto round 'P8' (2.4537 -1.5);\n");

	// holes for pogo pins
	fprintf (fout, "change drill 0.5mm\n");
	CalculateSmdPadXY (0, &x, &y);
	fprintf (fout, "pad auto round 'P1' (%8.3f %8.3f);\n", x, y);
	CalculateSmdPadXY (1, &x, &y);
	fprintf (fout, "pad auto round 'P2'  (%8.3f %8.3f);\n", x, y);
	CalculateSmdPadXY (2, &x, &y);
	fprintf (fout, "pad auto round 'P3'  (%8.3f %8.3f);\n", x, y);
	CalculateSmdPadXY (3, &x, &y);
	fprintf (fout, "pad auto round 'P4' (%8.3f %8.3f);\n", x, y);
	x = 0;
	y = -1.584;
	fprintf (fout, "pad auto round 'P5'  (%8.3f %8.3f);\n", x, y);

	// package name
	fprintf (fout, "change font vector\n");
	fprintf (fout, "change size 32mil\n");
	fprintf (fout, "change ratio 16%%\n");
	fprintf (fout, "change align top-center\n");
	fprintf (fout, "change layer tNames\n");
	fprintf (fout, "text '>NAME' (0 -3)\n");

	// part number, instructions (overlaps name)
	fprintf (fout, "change font vector\n");
	fprintf (fout, "change size 16mil\n");
	fprintf (fout, "change ratio 8%%\n");
	fprintf (fout, "change align top-center\n");
	fprintf (fout, "change layer tDocu\n");
	fprintf (fout, "text 'ED1486-ND' (0 -3.0)\n");
	fprintf (fout, "text '0985-2-15-20-71-14-11-0' (0 -3.5)\n");
	fprintf (fout, "text 'SOLDER PINS THIS SIDE' (0 -4.0)\n");
	fprintf (fout, "change layer bDocu\n");
	fprintf (fout, "text 'INSERT PINS THIS SIDE' (0 -4.0)\n");

	//----------------------------------------
	// lock footprint
	//----------------------------------------

	// package name
	fprintf (fout, "edit 'PAWPAD-LOCK.pac'\n");

	// top silkscreen artwork
	fprintf (fout, "change layer tPlace\n");
	fprintf (fout, "change width 4mil\n");
	RenderEaglePawPad (fout, false, true);

	// alignment holes, on equilateral triangle
	fprintf (fout, "change drill 1.6mm\n");
	fprintf (fout, "pad auto round 'P6' (-2.4537 -1.5);\n");
	fprintf (fout, "pad auto round 'P7' (0 2.75);\n");
	fprintf (fout, "pad auto round 'P8' (2.4537 -1.5);\n");

	// board outline
	fprintf (fout, "change layer dimension\n");
	fprintf (fout, "Wire 1mil (-2.598 -3.5) (2.598 -3.5);\n");
	fprintf (fout, "Wire 1mil (4.33 -0.5) (1.732 4);\n");
	fprintf (fout, "Wire 1mil (-1.732 4) (-4.33 -0.5);\n");
	fprintf (fout, "Wire 1mil (-1.732 4) -120 (1.732 4);\n");
	fprintf (fout, "Wire 1mil (-2.598 -3.5) -120 (-4.33 -0.5);\n");
	fprintf (fout, "Wire 1mil (4.33 -0.5) -120 (2.598 -3.5);\n");

	//----------------------------------------
	// 5 pin symbol
	//----------------------------------------

	fprintf (fout, "Edit 'PAWPAD.sym';\n");
	fprintf (fout, "set wire_bend 2;\n");
	fprintf (fout, "Pin 'P$1' io none middle R0 both 0 (-15.24 5.08);\n");
	fprintf (fout, "Pin 'P$2' io none middle R0 both 0 (-15.24 2.54);\n");
	fprintf (fout, "Pin 'P$3' io none middle R0 both 0 (-15.24 0);\n");
	fprintf (fout, "Pin 'P$4' io none middle R0 both 0 (-15.24 -2.54);\n");
	fprintf (fout, "Pin 'P$5' io none middle R0 both 0 (-15.24 -5.08);\n");
	fprintf (fout, "Layer 94;\n");
	fprintf (fout, "Change Style continuous;\n");
	fprintf (fout, "Wire  0.254 (-10.16 7.62) (-10.16 -7.62) (5.08 -7.62) (5.08 7.62) \\\n");
	fprintf (fout, "(-10.16 7.62);\n");
	fprintf (fout, "Layer 94;\n");
	fprintf (fout, "Change Size 70mil;\n");
	fprintf (fout, "Change Align bottom left;\n");
	fprintf (fout, "Change Font Proportional;\n");
	fprintf (fout, "Text '>NAME' R0 (-10.16 -10.16);\n");
	fprintf (fout, "Layer 94;\n");
	fprintf (fout, "Text '>VALUE' R0 (-10.16 -12.7);\n");
	fprintf (fout, "Layer 97;\n");
	fprintf (fout, "change width 1mil\n");
	RenderEaglePawPad (fout, false, false);
	fprintf (fout, "Change Align center;\n");
	fprintf (fout, "Change Size 24mil;\n");
	fprintf (fout, "Layer 94;\n");
	CalculateSmdPadXY (0, &x, &y);
	fprintf (fout, "Text '1' R0 (%8.3f %8.3f);\n", x, y);
	CalculateSmdPadXY (1, &x, &y);
	fprintf (fout, "Text '2' R0 (%8.3f %8.3f);\n", x, y);
	CalculateSmdPadXY (2, &x, &y);
	fprintf (fout, "Text '3' R0 (%8.3f %8.3f);\n", x, y);
	CalculateSmdPadXY (3, &x, &y);
	fprintf (fout, "Text '4' R0 (%8.3f %8.3f);\n", x, y);
	x = 0;
	y = -1.584;
	fprintf (fout, "Text '5' R0 (%8.3f %8.3f);\n", x, y);

	//----------------------------------------
	// devices
	//----------------------------------------

	fprintf (fout, "Edit 'PAWPAD-TARGET.dev';\n");
	fprintf (fout, "Prefix 'TP';\n");
	fprintf (fout, "Description 'pawpad target testpoint';\n");
	fprintf (fout, "Value off;\n");
	fprintf (fout, "Add PAWPAD 'G$1' next 0 (0 0);\n");
	fprintf (fout, "Package 'PAWPAD-TARGET' '';\n");
	fprintf (fout, "Technology '';\n");
	fprintf (fout, "Connect  'G$1.P$1' 'P1'  'G$1.P$2' 'P2'  'G$1.P$3' 'P3'  'G$1.P$4' 'P4'  'G$1.P$5' 'P5';\n");

	fprintf (fout, "Edit 'PAWPAD-FIXTURE.dev';\n");
	fprintf (fout, "Prefix 'TP';\n");
	fprintf (fout, "Description 'pawpad test fixture testpoint';\n");
	fprintf (fout, "Value off;\n");
	fprintf (fout, "Add PAWPAD 'G$1' next 0 (0 0);\n");
	fprintf (fout, "Package 'PAWPAD-FIXTURE' '';\n");
	fprintf (fout, "Technology '';\n");
	fprintf (fout, "Connect  'G$1.P$1' 'P1'  'G$1.P$2' 'P2'  'G$1.P$3' 'P3'  'G$1.P$4' 'P4'  'G$1.P$5' 'P5';\n");


	fclose (fout);

	return 0;
}


//---------------------------------------------------------------------------------------------
// RenderEaglePawPad
//

void RenderEaglePawPad (FILE *fout, bool stopLayer, bool wire)
{
	int pad, index;
	double x0, y0, x1, y1, x2, y2, x3, y3;
	double t, x, y;
	double cx, cy, sscale, xpos, ypos;

	for (pad = 0; pad < 5; pad++) {
		index = 0;
		if (wire) {
			fprintf (fout, "wire\n");
		} else {
			fprintf (fout, "polygon\n");
		}
		
		if (stopLayer) {
			cx = padcenters[pad][0];      // center x of artwork
			cy = padcenters[pad][1];      // center y of artwork
			sscale = padcenters[pad][2];  // scale
			xpos =  SCALE*(cx - XOFFSET); // center x of canvas
			ypos = -SCALE*(cy - YOFFSET); // center y of canvas
		} else {
			cx = XOFFSET;
			cy = YOFFSET;
			sscale = 1.0;
			xpos = 0;
			ypos = 0;
		}

		while (index <= (13-4)) {

			// parse control points

			x0 =  sscale*SCALE*(pads[pad][index][0] - cx);
			y0 = -sscale*SCALE*(pads[pad][index][1] - cy);
			index++;
	
			x1 =  sscale*SCALE*(pads[pad][index][0] - cx);
			y1 = -sscale*SCALE*(pads[pad][index][1] - cy);
			index++;
	
			x2 =  sscale*SCALE*(pads[pad][index][0] - cx);
			y2 = -sscale*SCALE*(pads[pad][index][1] - cy);
			index++;
	
			x3 =  sscale*SCALE*(pads[pad][index][0] - cx);
			y3 = -sscale*SCALE*(pads[pad][index][1] - cy);

			for (t = 0; t < 1; t += 1.0/25.0) {
				x = pow(1-t,3)*x0 + 3*t*pow(1-t,2)*x1 + 3*pow(t,2)*(1-t)*x2 + pow(t,3)*x3;
				y = pow(1-t,3)*y0 + 3*t*pow(1-t,2)*y1 + 3*pow(t,2)*(1-t)*y2 + pow(t,3)*y3;
				fprintf (fout, "(%10.3fmm %10.3fmm)\n", xpos+x, ypos+y);
			}
		}
		fprintf (fout, "(%10.3fmm %10.3fmm)\n", xpos+x3, ypos+y3);
	}
}


//---------------------------------------------------------------------------------------------
// CalculateSmdPadXY
//

void CalculateSmdPadXY (int index, double *x, double *y)
{
	// calculate center of pad from left top width height
	*x = SCALE*(padcenters[index][0] - XOFFSET); 
	*y = -SCALE*(padcenters[index][1] - YOFFSET);
	// printf ("pad %d at (%10.3f %10.3f)\n", index, *x, *y);
}
