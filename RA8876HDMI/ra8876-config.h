//--------------------------------------------------------------------------
// Copyright 2024, RIoT Secure AB
//
// @author Aaron Ardiri
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// RA8876 :: CONFIGURATION
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// RA8876RamInfo

RA8876RamInfo defaultRamInfo =
{
  165000,            // 165 MHz
  3,                 // CAS latency 3
  8 * 1024 * 1024L   // 64M (8mb)
};

//--------------------------------------------------------------------------
// RA8876DisplayInfo
//
// https://projectf.io/posts/video-timings-vga-720p-1080p
// https://tomverbeure.github.io/video_timings_calculator


#if   RA8876_CONFIG == RA8876_CONFIG_1366x768

//
// 1366x768 (DMT)
//
RA8876DisplayInfo defaultDisplayInfo =
{
  1366,   // display width
  768 ,   // display height
  72000,  // pixel clock in kHz

  14,     // horizontal front porch
  64,     // horizontal back porch
  56,     // HSYNC pulse width
  1,      // +hsync polarity

  1,      // vertical front porch
  28,     // vertical back porch
  3,      // VSYNC pulse width
  1       // +vsync polarity
};

#elif RA8876_CONFIG == RA8876_CONFIG_1280x720

//
// 1280x720 (CEA-861)
//
RA8876DisplayInfo defaultDisplayInfo =
{
  1280,   // Display width
  720,    // Display height
  74250,  // Pixel clock in kHz

  110,    // Horizontal front porch
  220,    // Horizontal back porch
  40,     // HSYNC pulse width
  1,      // +hsync polarity

  5,      // Vertical front porch
  20,     // Vertical back porch
  5,      // VSYNC pulse width
  1       // +vsync polarity
};

/*
//
// 1280x720 (CVT)
//
RA8876DisplayInfo defaultDisplayInfo =
{
  1280,   // Display width
  720,    // Display height
  74500,  // Pixel clock in kHz

  64,     // Horizontal front porch
  192,    // Horizontal back porch
  128,    // HSYNC pulse width
  0,      // -hsync polarity

  3,      // Vertical front porch
  20,     // Vertical back porch
  5,      // VSYNC pulse width
  1       // +vsync polarity
};

//
// 1280x720 (CVT-RB)
//
RA8876DisplayInfo defaultDisplayInfo =
{
  1280,   // Display width
  720,    // Display height
  64000,  // Pixel clock in kHz

  48,     // Horizontal front porch
  80,     // Horizontal back porch
  32,     // HSYNC pulse width
  1,      // +hsync polarity

  3,      // Vertical front porch
  13,     // Vertical back porch
  5,      // VSYNC pulse width
  1       // +vsync polarity (should be -1)
};
*/

#elif RA8876_CONFIG == RA8876_CONFIG_1024x768

//
// 1024x768 (CVT)
//
RA8876DisplayInfo defaultDisplayInfo =
{
  1024,   // Display width
  768,    // Display height
  63500,  // Pixel clock in kHz

  48,     // Horizontal front porch
  152,    // Horizontal back porch
  104,    // HSYNC pulse width
  0,      // -hsync polarity

  3,      // Vertical front porch
  23,     // Vertical back porch
  4,      // VSYNC pulse width
  1       // +vsync polarity
};

/*
//
// 1024x768 (CVT-RB)
//
RA8876DisplayInfo defaultDisplayInfo =
{
  1024,   // Display width
  768,    // Display height
  56000,  // Pixel clock in kHz

  48,     // Horizontal front porch
  80,     // Horizontal back porch
  32,     // HSYNC pulse width
  1,      // +hsync polarity

  3,      // Vertical front porch
  15,     // Vertical back porch
  4,      // VSYNC pulse width
  1       // -vsync polarity   (should be 0)
};

//
// 1024x768 (DMT)
//
RA8876DisplayInfo defaultDisplayInfo =
{
  1024,   // Display width
  768,    // Display height
  56000,  // Pixel clock in kHz

  24,     // Horizontal front porch
  160,    // Horizontal back porch
  136,    // HSYNC pulse width
  0,      // -hsync polarity

  3,      // Vertical front porch
  29,     // Vertical back porch
  6,      // VSYNC pulse width
  0       // -vsync polarity
};
*/

#elif RA8876_CONFIG == RA8876_CONFIG_1024x600

//
// 1024x600 (CVT)
//
RA8876DisplayInfo defaultDisplayInfo =
{
  1024,   // Display width
  600,    // Display height
  49000,  // Pixel clock in kHz

  40,     // Horizontal front porch
  144,    // Horizontal back porch
  104,    // HSYNC pulse width
  0,      // -hsync polarity

  3,      // Vertical front porch
  11,     // Vertical back porch
  10,     // VSYNC pulse width
  1       // +vsync polarity
};

/*
//
// 1024x600 (CVT-RB)
//
RA8876DisplayInfo defaultDisplayInfo =
{
  1024,   // Display width
  600,    // Display height
  43750,  // Pixel clock in kHz

  48,     // Horizontal front porch
  80,     // Horizontal back porch
  32,     // HSYNC pulse width
  1,      // +hsync polarity

  3,      // Vertical front porch
  6,      // Vertical back porch
  10,     // VSYNC pulse width
  1       // -vsync polarity   (should be -1)
};
*/

#elif RA8876_CONFIG == RA8876_CONFIG_800x600

/*
//
// 800x600 (CVT)
//
RA8876DisplayInfo defaultDisplayInfo =
{
  800,    // display width
  600,    // display height
  38250,  // pixel clock in kHz

  32,     // horizontal front porch
  80,     // horizontal back porch
  112,    // HSYNC pulse width
  0,      // -hsync polarity

  3,      // vertical front porch
  17,     // vertical back porch
  4,      // VSYNC pulse width
  1       // +vsync polarity
};

//
// 800x600 (CVT-RB)
//
RA8876DisplayInfo defaultDisplayInfo =
{
  800,    // display width
  600,    // display height
  35500,  // pixel clock in kHz

  48,     // horizontal front porch
  80,     // horizontal back porch
  32,     // HSYNC pulse width
  1,      // +hsync polarity

  3,      // vertical front porch
  11,     // vertical back porch
  4,      // VSYNC pulse width
  1       // -vsync polarity   (should be 0)
};
*/

//
// 800x600 (DMT)
//
RA8876DisplayInfo defaultDisplayInfo =
{
  800,    // display width
  600,    // display height
  40000,  // pixel clock in kHz

  40,     // horizontal front porch
  88,     // horizontal back porch
  128,    // HSYNC pulse width
  1,      // +hsync polarity

  1,      // vertical front porch
  23,     // vertical back porch
  4,      // VSYNC pulse width
  1       // +vsync polarity
};

#else
#error RA8876_CONFIG not defined
#endif

//--------------------------------------------------------------------------
