//--------------------------------------------------------------------------
// Copyright 2024, RIoT Secure AB
//
// @author Aaron Ardiri
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// RA8876 :: FOUNDATIONS
//--------------------------------------------------------------------------

// datasheet
// https://www.raio.com.tw/data_raio/Datasheet/RA88%20Series/Simple_2022/RA8876M_Brief_DS_V10_Eng.pdf

//--------------------------------------------------------------------------
// Color

class Color
{
  public:
    Color();
    Color(uint8_t red, uint8_t green, uint8_t blue);
    explicit Color(uint32_t color);
    uint8_t  toRGB332() const;
    uint16_t toRGB565() const;

    static const Color Black;       // black
    static const Color White;       // white
    static const Color Red;         // red
    static const Color Green;       // green
    static const Color Blue;        // blue
    static const Color Yellow;      // yellow
    static const Color Magenta;     // magenta
    static const Color Cyan;        // cyan
    static const Color Orange;      // orange
    static const Color Pink;        // pink

    uint8_t r; // red component
    uint8_t g; // green component
    uint8_t b; // blue component
};

Color::Color() : r(0), g(0), b(0)
{
}

Color::Color(uint8_t red, uint8_t green, uint8_t blue) 
  : r(red), g(green), b(blue)
{
}

Color::Color(uint32_t color) 
  : r((color & 0x00ff0000) >> 16), g((color & 0x0000ff00) >> 8), b((color & 0x000000ff))
{
}

uint8_t Color::toRGB332() const
{
  return (( (uint8_t)r & 0xe0)       | 
          (((uint8_t)g & 0xe0) >> 3) | 
           ((uint8_t)b & 0xc0) >> 6);
}

uint16_t Color::toRGB565() const
{
  return ((((uint16_t)r & 0xf8) << 8) | 
          (((uint16_t)g & 0xfc) << 3) | 
          (((uint16_t)b & 0xf8) >> 3));
}

// static color defines
const Color Color::Black      (0x00, 0x00, 0x00);
const Color Color::White      (0xff, 0xff, 0xff);
const Color Color::Red        (0xff, 0x00, 0x00);
const Color Color::Green      (0x00, 0xff, 0x00);
const Color Color::Blue       (0x00, 0x00, 0xff);
const Color Color::Yellow     (0xff, 0xff, 0x00);
const Color Color::Magenta    (0xff, 0x00, 0xff);
const Color Color::Cyan       (0x00, 0xff, 0xff);
const Color Color::Orange     (0xff, 0xa5, 0x00);
const Color Color::Pink       (0xff, 0x3a, 0x6a);

//--------------------------------------------------------------------------
// RA8876RamInfo

struct RA8876RamInfo
{
  uint32_t freq;        // kHz
  uint8_t  latency;     // CAS latency (2 or 3)
  uint32_t sz;
};

//--------------------------------------------------------------------------
// RA8876DisplayInfo

struct RA8876DisplayInfo
{
  uint16_t width;       // display width
  uint16_t height;      // display height

  uint32_t dotClock;    // pixel clock in kHz

  uint16_t hFrontPorch; // (rounded to the nearest multiple of 8)
  uint16_t hBackPorch;
  uint16_t hPulseWidth; // (rounded to the nearest multiple of 8)
  int8_t   hSyncPolarity;

  uint16_t vFrontPorch;
  uint16_t vBackPorch;
  uint16_t vPulseWidth;
  int8_t   vSyncPolarity;
};

//--------------------------------------------------------------------------
// RA8876PllParams

struct RA8876PllParams
{
  uint32_t freq;        // frequency in kHz
  int      m;           // 0 or 1
  int      n;           // multiplier less 1 (range 1..63)
  int      k;           // divisor power of 2 (range 0..3 for CCLK/MCLK; range 0..7 for SCLK)
};

enum RA8876FontSize
{
  RA8876_FONT_SIZE_16                   = 0x00,
  RA8876_FONT_SIZE_24                   = 0x01,
  RA8876_FONT_SIZE_32                   = 0x02
};

enum RA8876FontEncoding
{
  RA8876_FONT_ENCODING_GB2312           = 0x00,  // GB2312 (Simplified Chinese)
  RA8876_FONT_ENCODING_GB18030          = 0x01,  // GB12345/GB18030 (Chinese)
  RA8876_FONT_ENCODING_BIG5             = 0x02,  // Big5 (Traditional Chinese)
  RA8876_FONT_ENCODING_UNICODE          = 0x03,  // Unicode (UCS-2?)
  RA8876_FONT_ENCODING_ASCII            = 0x04,  // ASCII
  RA8876_FONT_ENCODING_UNIJAPAN         = 0x05,  // Uni-Japanese (?)
  RA8876_FONT_ENCODING_JIS0208          = 0x06,  // JIS X 0208 (Shift JIS?)
  RA8876_FONT_ENCODING_LGCATH           = 0x07,  // Latin/Greek/Cyrillic/Arabic/Thai/Hebrew (?)
  RA8876_FONT_ENCODING_8859_1           = 0x11,  // ISO 8859-1  (Latin 1)
  RA8876_FONT_ENCODING_8859_2           = 0x12,  // ISO 8859-2  (Latin 2: Eastern European)
  RA8876_FONT_ENCODING_8859_3           = 0x13,  // ISO 8859-3  (Latin 3: South European)
  RA8876_FONT_ENCODING_8859_4           = 0x14,  // ISO 8859-4  (Latin 4: Northern European)
  RA8876_FONT_ENCODING_8859_5           = 0x15,  // ISO 8859-5  (Latin/Cyrillic)
  RA8876_FONT_ENCODING_8859_7           = 0x16,  // ISO 8859-7  (Latin/Greek)
  RA8876_FONT_ENCODING_8859_8           = 0x17,  // ISO 8859-8  (Latin/Hebrew)
  RA8876_FONT_ENCODING_8859_9           = 0x18,  // ISO 8859-9  (Latin 5: Turkish)
  RA8876_FONT_ENCODING_8859_10          = 0x19,  // ISO 8859-10 (Latin 6: Nordic)
  RA8876_FONT_ENCODING_8859_11          = 0x1A,  // ISO 8859-11 (Latin/Thai)
  RA8876_FONT_ENCODING_8859_13          = 0x1B,  // ISO 8859-13 (Latin 7: Baltic Rim)
  RA8876_FONT_ENCODING_8859_14          = 0x1C,  // ISO 8859-14 (Latin 8: Celtic)
  RA8876_FONT_ENCODING_8859_15          = 0x1D,  // ISO 8859-15 (Latin 9: Western European)
  RA8876_FONT_ENCODING_8859_16          = 0x1E   // ISO 8859-16 (Latin 10: South-Eastern European)
};

typedef uint8_t RA8876FontFlags;

#define RA8876_FONT_FLAG_XLAT_FULLWIDTH   0x01  // translate ASCII to Unicode fullwidth forms

#define RA8876_DATA_WRITE                 0x80
#define RA8876_DATA_READ                  0xC0
#define RA8876_CMD_WRITE                  0x00

// Data sheet 19.1: status register
#define RA8876_STATUS_READ                0x40

  #define RA8876_STATUS_HMWFF_MASK        (1 << 7) // host memory write FIFO full mask
  #define RA8876_STATUS_HMWFF_F             (1 << 7) // host memory write FIFO full
  #define RA8876_STATUS_HMWFF_NF            (0 << 7) // host memory write FIFO not full

  #define RA8876_STATUS_HMWFE_MASK        (1 << 6) // host memory write FIFO empty mask
  #define RA8876_STATUS_HMWFE_E             (1 << 6) // host memory write FIFO empty 
  #define RA8876_STATUS_HMWFE_NE            (0 << 6) // host memory write FIFO not empty

  #define RA8876_STATUS_HMRFF_MASK        (1 << 5) // host memory read FIFO full mask
  #define RA8876_STATUS_HMRFF_F             (1 << 5) // host memory read FIFO full
  #define RA8876_STATUS_HMRFF_NF            (0 << 5) // host memory read FIFO not full

  #define RA8876_STATUS_HMRFE_MASK        (1 << 4) // host memory read FIFO empty mask
  #define RA8876_STATUS_HMRFE_E             (1 << 4) // host memory read FIFO empty
  #define RA8876_STATUS_HMRFE_NE            (0 << 4) // host memory read FIFO not empty

  #define RA8876_STATUS_TASK_MASK         (1 << 3) // task state mask
  #define RA8876_STATUS_TASK_BUSY           (1 << 3) // task busy
  #define RA8876_STATUS_TASK_IDLE           (0 << 3) // task done

  #define RA8876_STATUS_BRAM_MASK         (1 << 2) // buffer RAM access mask
  #define RA8876_STATUS_BRAM_READY          (1 << 2) // buffer RAM ready for access
  #define RA8876_STATUS_BRAM_BUSY           (0 << 2) // buffer RAM not ready for access

  #define RA8876_STATUS_MODE_MASK         (1 << 1) // operation mode mask
  #define RA8876_STATUS_MODE_WAIT           (1 << 1) // inhibit operation state
  #define RA8876_STATUS_MODE_NORM           (0 << 1) // normal operation state

  #define RA8876_STATUS_ISR_MASK          (1 << 0) // interrupt state mask
  #define RA8876_STATUS_ISR_ACTIVE          (1 << 0) // interupt active
  #define RA8876_STATUS_ISR_IDLE            (0 << 0) // without interupt active

// Data sheet 19.2: chip configuration registers
#define RA8876_REG_SRR                    0x00  // software reset register

  #define RA8876_REG_SRR_RESET              (1 << 1) // software reset
  #define RA8876_REG_SRR_WARNING            (1 << 0) // warning condition occurred

#define RA8876_REG_CCR                    0x01  // chip configuration register

  #define RA8876_REG_CCR_CONF_PLL           (1 << 7) // reconfigure PLL

  #define RA8876_REG_CCR_TFT_MASK         (3 << 3) // TFT panel output setting
  #define RA8876_REG_CCR_TFT_24BIT          (0 << 3)
  #define RA8876_REG_CCR_TFT_18BIT          (1 << 3)
  #define RA8876_REG_CCR_TFT_16BIT          (2 << 3)
  #define RA8876_REG_CCR_TFT_NONE           (3 << 3)

#define RA8876_REG_MACR                   0x02  // memory access control register

  #define RA8876_REG_MACR_MPU_MASK        (3 << 6) // host read/write image format
  #define RA8876_REG_MACR_MPU_MHBE          (3 << 6) // mask high byte even
  #define RA8876_REG_MACR_MPU_MHBA          (2 << 6) // mask high byte all
  #define RA8876_REG_MACR_MPU_DIRECT        (0 << 6) // direct write

  #define RA8876_REG_MACR_DIRR_MASK       (3 << 4) // host read memory direction
  #define RA8876_REG_MACR_DIRR_BTLR         (3 << 4) // buttom-top-left-right
  #define RA8876_REG_MACR_DIRR_TBLR         (2 << 4) // top-bottom-left-right
  #define RA8876_REG_MACR_DIRR_RLTB         (1 << 4) // right-left-top-bottom
  #define RA8876_REG_MACR_DIRR_LRTB         (0 << 4) // left-right-top-bottom

  #define RA8876_REG_MACR_DIRW_MASK       (3 << 1) // host write memory direction
  #define RA8876_REG_MACR_DIRW_BTLR         (3 << 1) // buttom-top-left-right
  #define RA8876_REG_MACR_DIRW_TBLR         (2 << 1) // top-bottom-left-right
  #define RA8876_REG_MACR_DIRW_RLTB         (1 << 1) // right-left-top-bottom
  #define RA8876_REG_MACR_DIRW_LRTB         (0 << 1) // left-right-top-bottom

#define RA8876_REG_ICR                    0x03  // input control register

  #define RA8876_REG_ICR_MODE_MASK        (1 << 2) // text mode enable
  #define RA8876_REG_ICR_MODE_TEXT          (1 << 2) // text mode
  #define RA8876_REG_ICR_MODE_GRAPHIC       (0 << 2) // graphics mode

  #define RA8876_REG_ICR_MEM_MASK         (3 << 0) // memory port read/write
  #define RA8876_REG_ICR_MEM_CPRAM          (3 << 0) // color palette ram
  #define RA8876_REG_ICR_MEM_GCRAM          (2 << 0) // graphics cursor ram
  #define RA8876_REG_ICR_MEM_GT             (1 << 0) // gamma table
  #define RA8876_REG_ICR_MEM_BRAM           (0 << 0) // buffer ram

#define RA8876_REG_MRWDP                  0x04  // memory read/write data port

// Data sheet 19.3: PLL setting registers
#define RA8876_REG_PPLLC1                 0x05  // SCLK PLL control register 1
#define RA8876_REG_PPLLC2                 0x06  // SCLK PLL control register 2
#define RA8876_REG_MPLLC1                 0x07  // MCLK PLL control register 1
#define RA8876_REG_MPLLC2                 0x08  // MCLK PLL control register 2
#define RA8876_REG_SPLLC1                 0x09  // CCLK PLL control register 1
#define RA8876_REG_SPLLC2                 0x0A  // CCLK PLL control register 2

                                       // 0x0B
                                       // ..
                                       // 0x0F  // undefined/reserved

// Data sheet 19.5: LCD display control registers
#define RA8876_REG_MPWCTR                 0x10  // main/PIP window control register

  #define RA8876_REG_MPWCTR_PIP1_MASK     (1 << 7) // PIP1 enable/disable
  #define RA8876_REG_MPWCTR_PIP1_ENABLE     (1 << 7) // PIP1 enable
  #define RA8876_REG_MPWCTR_PIP1_DISABLE    (0 << 7) // PIP2 disable

  #define RA8876_REG_MPWCTR_PIP2_MASK     (1 << 6) // PIP2 enable/disable
  #define RA8876_REG_MPWCTR_PIP2_ENABLE     (1 << 6) // PIP1 enable
  #define RA8876_REG_MPWCTR_PIP2_DISABLE    (0 << 6) // PIP2 disable

  #define RA8876_REG_MPWCTR_PIPC_MASK     (1 << 4) // select PIP configure
  #define RA8876_REG_MPWCTR_PIPC_PIP2       (1 << 4) // PIP2
  #define RA8876_REG_MPWCTR_PIPC_PIP1       (0 << 4) // PIP1

  #define RA8876_REG_MPWCTR_DEPTH_MASK    (3 << 2) // main image depth
  #define RA8876_REG_MPWCTR_DEPTH_24BPP     (2 << 2) // 24bpp
  #define RA8876_REG_MPWCTR_DEPTH_16BPP     (1 << 2) // 16bpp
  #define RA8876_REG_MPWCTR_DEPTH_8BPP      (0 << 2) // 8bpp

  #define RA8876_REG_MPWCTR_SYNC_MASK     (1 << 0) // sync signals
  #define RA8876_REG_MPWCTR_SYNC_IDLE       (1 << 0) // idle
  #define RA8876_REG_MPWCTR_SYNC_ENABLE     (0 << 0) // enable

#define RA8876_REG_PIPCDEP                0x11  // PIP window color depth register
#define RA8876_REG_DPCR                   0x12  // display configuration register

  #define RA8876_REG_DPCR_PCLK_MASK       (1 << 7) // PCLK inversion
  #define RA8876_REG_DPCR_PCLK_FALL         (1 << 7) // fetch on PCLK falling edge
  #define RA8876_REG_DPCR_PCLK_RISE         (0 << 7) // fetch on PCLK rising edge

  #define RA8876_REG_DPCR_DISPLAY_MASK    (1 << 6) // display ON/OFF
  #define RA8876_REG_DPCR_DISPLAY_ON        (1 << 6) // display on
  #define RA8876_REG_DPCR_DISPLAY_OFF       (0 << 6) // display off

  #define RA8876_REG_DPCR_TEST_MASK       (1 << 5) // display TEST color bar
  #define RA8876_REG_DPCR_TEST_ENABLE       (1 << 5) // enable
  #define RA8876_REG_DPCR_TEST_DISABLE      (0 << 5) // disable

  #define RA8876_REG_DPCR_VDIR_MASK       (1 << 3) // vertical scan direction
  #define RA8876_REG_DPCR_VDIR_BT           (1 << 3) // bottom to top
  #define RA8876_REG_DPCR_VDIR_TB           (0 << 3) // top to bottom

  #define RA8876_REG_DPCR_XPDA_MASK       (7 << 0) // output sequence
  #define RA8876_REG_DPCR_XPDA_IDLE         (7 << 0) // idle state
  #define RA8876_REG_DPCR_XPDA_GRAY         (6 << 0) // grayscale
  #define RA8876_REG_DPCR_XPDA_BGR          (5 << 0)
  #define RA8876_REG_DPCR_XPDA_BRG          (4 << 0)
  #define RA8876_REG_DPCR_XPDA_GBR          (3 << 0)
  #define RA8876_REG_DPCR_XPDA_GRB          (2 << 0)
  #define RA8876_REG_DPCR_XPDA_RBG          (1 << 0)
  #define RA8876_REG_DPCR_XPDA_RGB          (0 << 0)

#define RA8876_REG_PCSR                   0x13  // panel scan clock and data setting register

  #define RA8876_REG_PCSR_XHSYNC_MASK     (1 << 7)
  #define RA8876_REG_PCSR_XHSYNC_HIGH       (1 << 7)
  #define RA8876_REG_PCSR_XHSYNC_LOW        (0 << 7)

  #define RA8876_REG_PCSR_XVSYNC_MASK     (1 << 6)
  #define RA8876_REG_PCSR_XVSYNC_HIGH       (1 << 6)
  #define RA8876_REG_PCSR_XVSYNC_LOW        (0 << 6)

  #define RA8876_REG_PCSR_XDE_MASK        (1 << 5)
  #define RA8876_REG_PCSR_XDE_LOW           (1 << 5)
  #define RA8876_REG_PCSR_XDE_HIGH          (0 << 5)

  #define RA8876_REG_PCSR_XDEI_MASK       (1 << 4)
  #define RA8876_REG_PCSR_XDEI_HIGH         (1 << 4)
  #define RA8876_REG_PCSR_XDEI_LOW          (0 << 4)

  #define RA8876_REG_PCSR_XPCLKI_MASK     (1 << 3)
  #define RA8876_REG_PCSR_XPCLKI_HIGH       (1 << 3)
  #define RA8876_REG_PCSR_XPCLKI_LOW        (0 << 3)

  #define RA8876_REG_PCSR_XPDATI_MASK     (1 << 2)
  #define RA8876_REG_PCSR_XPDATI_HIGH       (1 << 2)
  #define RA8876_REG_PCSR_XPDATI_LOW        (0 << 2)

  #define RA8876_REG_PCSR_XHSYNCI_MASK    (1 << 1)
  #define RA8876_REG_PCSR_XHSYNCI_HIGH      (1 << 1)
  #define RA8876_REG_PCSR_XHSYNCI_LOW       (0 << 1)

  #define RA8876_REG_PCSR_XVSYNCI_MASK    (1 << 0)
  #define RA8876_REG_PCSR_XVSYNCI_HIGH      (1 << 0)
  #define RA8876_REG_PCSR_XVSYNCI_LOW       (0 << 0)

#define RA8876_REG_HDWR                   0x14  // horizontal display width register
#define RA8876_REG_HDWFTR                 0x15  // horizontal display width fine tuning register
#define RA8876_REG_HNDR                   0x16  // horizontal non-display period register
#define RA8876_REG_HNDFTR                 0x17  // horizontal non-display period fine tuning register
#define RA8876_REG_HSTR                   0x18  // HSYNC start position register
#define RA8876_REG_HPWR                   0x19  // HSYNC pulse width register
#define RA8876_REG_VDHR0                  0x1A  // vertical display height register 0
#define RA8876_REG_VDHR1                  0x1B  // vertical display height register 1
#define RA8876_REG_VNDR0                  0x1C  // vertical non-display period register 0
#define RA8876_REG_VNDR1                  0x1D  // vertical non-display period register 1
#define RA8876_REG_VSTR                   0x1E  // VSYNC start position register
#define RA8876_REG_VPWR                   0x1F  // VSYNC pulse width register
#define RA8876_REG_MISA0                  0x20  // main image start address 0
#define RA8876_REG_MISA1                  0x21  // main image start address 1
#define RA8876_REG_MISA2                  0x22  // main image start address 2
#define RA8876_REG_MISA3                  0x23  // main image start address 3
#define RA8876_REG_MIW0                   0x24  // main image width 0
#define RA8876_REG_MIW1                   0x25  // main image width 1
#define RA8876_REG_MWULX0                 0x26  // main window upper-left X coordinate 0
#define RA8876_REG_MWULX1                 0x27  // main window upper-left X coordinate 1
#define RA8876_REG_MWULY0                 0x28  // main window upper-left Y coordinate 0
#define RA8876_REG_MWULY1                 0x29  // main window upper-left Y coordinate 1

                                       // 0x2A 
                                       // .. 
                                       // 0x4F  // undefined/reserved

// Data sheet 19.6: Geometric engine control registers
#define RA8876_REG_CVSSA0                 0x50  // canvas Start Address 0
#define RA8876_REG_CVSSA1                 0x51  // canvas Start Address 1
#define RA8876_REG_CVSSA2                 0x52  // canvas Start Address 2
#define RA8876_REG_CVSSA3                 0x53  // canvas Start Address 3
#define RA8876_REG_CVS_IMWTH0             0x54  // canvas image width 0
#define RA8876_REG_CVS_IMWTH1             0x55  // canvas image width 1
#define RA8876_REG_AWUL_X0                0x56  // active window upper-left X coordinate 0
#define RA8876_REG_AWUL_X1                0x57  // active window upper-left X coordinate 1
#define RA8876_REG_AWUL_Y0                0x58  // active window upper-left Y coordinate 0
#define RA8876_REG_AWUL_Y1                0x59  // active window upper-left Y coordinate 1
#define RA8876_REG_AW_WTH0                0x5A  // active window width 0
#define RA8876_REG_AW_WTH1                0x5B  // active window width 1
#define RA8876_REG_AW_HT0                 0x5C  // active window height 0
#define RA8876_REG_AW_HT1                 0x5D  // active window height 1
#define RA8876_REG_AW_COLOR               0x5E  // color depth of canvas & active window

  #define RA8876_REG_AW_COLOR_ADDR_MASK   (1 << 2) // canvas addressing mode
  #define RA8876_REG_AW_COLOR_ADDR_LINEAR   (1 << 2) // linear
  #define RA8876_REG_AW_COLOR_ADDR_BLOCK    (0 << 2) // block (x, y) coordinates

  #define RA8876_REG_AW_COLOR_DEPTH_MASK  (3 << 0) // cavnas image depth
  #define RA8876_REG_AW_COLOR_DEPTH_24BPP   (2 << 0) // 24bpp
  #define RA8876_REG_AW_COLOR_DEPTH_16BPP   (1 << 0) // 16bpp
  #define RA8876_REG_AW_COLOR_DEPTH_8BPP    (0 << 0) // 8bpp

#define RA8876_REG_CURH0                  0x5F  // graphic read/write horizontal position 0
#define RA8876_REG_CURH1                  0x60  // graphic read/write horizontal position 1
#define RA8876_REG_CURV0                  0x61  // graphic read/write vertical position 0
#define RA8876_REG_CURV1                  0x62  // graphic read/write vertical position 1
#define RA8876_REG_F_CURX0                0x63  // text cursor X-coordinate register 0
#define RA8876_REG_F_CURX1                0x64  // text cursor X-coordinate register 1
#define RA8876_REG_F_CURY0                0x65  // text cursor Y-coordinate register 0
#define RA8876_REG_F_CURY1                0x66  // text cursor Y-coordinate register 1

#define RA8876_REG_DCR0                   0x67  // draw shape control register 0

#define RA8876_REG_DCR0_DRAW_LINE           0x80  // b1000 0000
#define RA8876_REG_DCR0_DRAW_TRIANGLE       0xA2  // b1100 0010
#define RA8876_REG_DCR0_DRAW_TRIANGLE_FILL  0xE2  // b1110 0010

#define RA8876_REG_DLHSR0                 0x68  // draw shape point 1 X coordinate register 0
#define RA8876_REG_DLHSR1                 0x69  // draw shape point 1 X coordinate register 1
#define RA8876_REG_DLVSR0                 0x6A  // draw shape point 1 Y coordinate register 0
#define RA8876_REG_DLVSR1                 0x6B  // draw shape point 1 Y coordinate register 1
#define RA8876_REG_DLHER0                 0x6C  // draw shape point 2 X coordinate register 0
#define RA8876_REG_DLHER1                 0x6D  // draw shape point 2 X coordinate register 1
#define RA8876_REG_DLVER0                 0x6E  // draw shape point 2 Y coordinate register 0
#define RA8876_REG_DLVER1                 0x6F  // draw shape point 2 Y coordinate register 1
#define RA8876_REG_DTPH0                  0x70  // draw shape point 3 X coordinate register 0
#define RA8876_REG_DTPH1                  0x71  // draw shape point 3 X coordinate register 1
#define RA8876_REG_DTPV0                  0x72  // draw shape point 3 Y coordinate register 0
#define RA8876_REG_DTPV1                  0x73  // draw shape point 3 Y coordinate register 1
#define RA8876_REG_DCR1                   0x76  // draw shape control register 1

#define RA8876_REG_DCR1_DRAW_CIRCLE         0x80  // b1000 0000
#define RA8876_REG_DCR1_DRAW_CIRCLE_FILL    0xC0  // b1100 0000
#define RA8876_REG_DCR1_DRAW_BL_CURVE       0x90  // b1001 0000
#define RA8876_REG_DCR1_DRAW_BL_CURVE_FILL  0xD0  // b1101 0000
#define RA8876_REG_DCR1_DRAW_UL_CURVE       0x91  // b1001 0001
#define RA8876_REG_DCR1_DRAW_UL_CURVE_FILL  0xD1  // b1101 0001
#define RA8876_REG_DCR1_DRAW_UR_CURVE       0x92  // b1001 0002
#define RA8876_REG_DCR1_DRAW_UR_CURVE_FILL  0xD2  // b1101 0002
#define RA8876_REG_DCR1_DRAW_BR_CURVE       0x93  // b1001 0003
#define RA8876_REG_DCR1_DRAW_BR_CURVE_FILL  0xD3  // b1101 0003
#define RA8876_REG_DCR1_DRAW_RECT           0xA0  // b1010 0000
#define RA8876_REG_DCR1_DRAW_RECT_FILL      0xE0  // b1110 0000
#define RA8876_REG_DCR1_DRAW_RRECT          0xB0  // b1011 0000
#define RA8876_REG_DCR1_DRAW_RRECT_FILL     0xF0  // b1111 0000
#define RA8876_REG_DCR1_DRAW_ELLIPSE        RA8876_REG_DCR1_DRAW_CIRCLE
#define RA8876_REG_DCR1_DRAW_ELLIPSE_FILL   RA8876_REG_DCR1_DRAW_CIRCLE_FILL

#define RA8876_REG_ELL_A0                 0x77  // draw ellipse major radius 0
#define RA8876_REG_ELL_A1                 0x78  // draw ellipse major radius 1
#define RA8876_REG_ELL_B0                 0x79  // draw ellipse minor radius 0
#define RA8876_REG_ELL_B1                 0x7A  // draw ellipse minor radius 1
#define RA8876_REG_DEHR0                  0x7B  // draw ellipse centre X coordinate register 0
#define RA8876_REG_DEHR1                  0x7C  // draw ellipse centre X coordinate register 1
#define RA8876_REG_DEVR0                  0x7D  // draw ellipse centre Y coordinate register 0
#define RA8876_REG_DEVR1                  0x7E  // draw ellipse centre Y coordinate register 1

                                       // 0x7F
                                       // ..
                                       // 0x83  // undefined/reserved

// Data sheet 19.7: PWM timer control registers
#define RA8876_REG_PSCLR                  0x84  // PWM prescaler register
#define RA8876_REG_PMUXR                  0x85  // PWM clock mux register
#define RA8876_REG_PCFGR                  0x86  // PWM configuration register
#define RA8876_REG_DZ_LENGTH              0x87  // timer 0 dead zone length register
#define RA8876_REG_TCMPB0L                0x88  // timer 0 compare buffer register
#define RA8876_REG_TCMPB0H                0x89  // timer 0 compare buffer register
#define RA8876_REG_TCNTB0L                0x8A  // timer 0 count buffer register
#define RA8876_REG_TCNTB0H                0x8B  // timer 0 count buffer register
#define RA8876_REG_TCMPB1L                0x8C  // timer 1 compare buffer register
#define RA8876_REG_TCMPB1H                0x8D  // timer 1 compare buffer register
#define RA8876_REG_TCNTB1L                0x8E  // timer 1 count buffer register
#define RA8876_REG_TCNTB1F                0x8F  // timer 1 count buffer register

// Data sheet 19.8: Block Transfer Engine (BTE) control registers
#define RA8876_REG_BTE_CTRL0              0x90

#define RA8876_REG_BTE_ENABLE               (1 << 4)
#define RA8876_REG_BTE_PATTERN_MASK       (1 << 1)
#define RA8876_REG_BTE_PATTERN_FORMAT16X16  (1 << 0)
#define RA8876_REG_BTE_PATTERN_FORMAT8X8    (0 << 0)

#define RA8876_REG_BTE_CTRL1              0x91

#define RA8876_REG_BTE_ROP_CODE_MASK        (15 << 4)
#define RA8876_REG_BTE_ROP_CODE_0           (0  << 4)   // 0 ( Blackness ) 
#define RA8876_REG_BTE_ROP_CODE_1           (1  << 4)   // ~S0*~S1 or ~ ( S0+S1 ) 
#define RA8876_REG_BTE_ROP_CODE_2           (2  << 4)   // ~S0*S1
#define RA8876_REG_BTE_ROP_CODE_3           (3  << 4)   // ~S0
#define RA8876_REG_BTE_ROP_CODE_4           (4  << 4)   // S0*~S1
#define RA8876_REG_BTE_ROP_CODE_5           (5  << 4)   // ~S1
#define RA8876_REG_BTE_ROP_CODE_6           (6  << 4)   // S0^S1
#define RA8876_REG_BTE_ROP_CODE_7           (7  << 4)   // ~S0+~S1 or ~ ( S0*S1 ) 
#define RA8876_REG_BTE_ROP_CODE_8           (8  << 4)   // S0*S1
#define RA8876_REG_BTE_ROP_CODE_9           (9  << 4)   // ~ ( S0^S1 ) 
#define RA8876_REG_BTE_ROP_CODE_10          (10 << 4)   // S1
#define RA8876_REG_BTE_ROP_CODE_11          (11 << 4)   // ~S0+S1
#define RA8876_REG_BTE_ROP_CODE_12          (12 << 4)   // S0
#define RA8876_REG_BTE_ROP_CODE_13          (13 << 4)   // S0+~S1
#define RA8876_REG_BTE_ROP_CODE_14          (14 << 4)   // S0+S1
#define RA8876_REG_BTE_ROP_CODE_15          (15 << 4)   // 1 ( Whiteness ) 
#define RA8876_REG_BTE_OPERATION_MASK     (15 << 0)
#define RA8876_REG_BTE_MPU_WR_ROP           (0  << 0)  
#define RA8876_REG_BTE_MEM_CPY_ROP          (2  << 0)
#define RA8876_REG_BTE_MPU_WR_CHROMA        (4  << 0)
#define RA8876_REG_BTE_MEM_CPY_CHROMA       (5  << 0)
#define RA8876_REG_BTE_PAT_FILL_ROP         (6  << 0)
#define RA8876_REG_BTE_PAT_FILL_CHROMA      (7  << 0)
#define RA8876_REG_BTE_MPU_WR_CEXP          (8  << 0)
#define RA8876_REG_BTE_MPU_WR_CEXP_CHROMA   (9  << 0)
#define RA8876_REG_BTE_MEM_CPY_OPACITY      (10 << 0)
#define RA8876_REG_BTE_MPU_WR_OPACITY       (11 << 0)
#define RA8876_REG_BTE_SOLID_FILL           (12 << 0)
#define RA8876_REG_BTE_MEM_CPY_CEXP         (14 << 0)
#define RA8876_REG_BTE_MEM_CPY_CEXP_CHROMA  (15 << 0)

#define RA8876_REG_BTE_COLR               0x92

#define RA8876_REG_BTE_S0_DEPTH_MASK      (3 << 5)
#define RA8876_REG_BTE_S0_DEPTH_8BPP        (0 << 5)
#define RA8876_REG_BTE_S0_DEPTH_16BPP       (1 << 5)
#define RA8876_REG_BTE_S0_DEPTH_24BPP       (2 << 5)
#define RA8876_REG_BTE_S1_DEPTH_MASK      (7 << 2)
#define RA8876_REG_BTE_S1_DEPTH_8BPP        (0 << 2)
#define RA8876_REG_BTE_S1_DEPTH_16BPP       (1 << 2)
#define RA8876_REG_BTE_S1_DEPTH_24BPP       (2 << 2)
#define RA8876_REG_BTE_S1_CONSTANT_COLOR    (3 << 2)
#define RA8876_REG_BTE_S1_8BIT_ABLEND       (4 << 2)
#define RA8876_REG_BTE_S1_16BIT_ABLEND      (5 << 2)
#define RA8876_REG_BTE_DEST_DEPTH_MASK    (3 << 0)
#define RA8876_REG_BTE_DEST_DEPTH_8BPP      (0 << 0)
#define RA8876_REG_BTE_DEST_DEPTH_16BPP     (1 << 0)
#define RA8876_REG_BTE_DEST_DEPTH_24BPP     (2 << 0)

#define RA8876_REG_BTE_S0_STR0            0x93
#define RA8876_REG_BTE_S0_STR1            0x94
#define RA8876_REG_BTE_S0_STR2            0x95
#define RA8876_REG_BTE_S0_STR3            0x96
#define RA8876_REG_BTE_S0_WTH0            0x97
#define RA8876_REG_BTE_S0_WTH1            0x98
#define RA8876_REG_BTE_S0_X0              0x99
#define RA8876_REG_BTE_S0_X1              0x9A
#define RA8876_REG_BTE_S0_Y0              0x9B
#define RA8876_REG_BTE_S0_Y1              0x9C

#define RA8876_REG_BTE_S1_STR0            0x9D
#define RA8876_REG_BTE_S1_STR1            0x9E
#define RA8876_REG_BTE_S1_STR2            0x9F
#define RA8876_REG_BTE_S1_RED             0x9D
#define RA8876_REG_BTE_S1_GREEN           0x9E
#define RA8876_REG_BTE_S1_BLUE            0x9F
#define RA8876_REG_BTE_S1_STR3            0xA0
#define RA8876_REG_BTE_S1_WTH0            0xA1
#define RA8876_REG_BTE_S1_WTH1            0xA2
#define RA8876_REG_BTE_S1_X0              0xA3
#define RA8876_REG_BTE_S1_X1              0xA4
#define RA8876_REG_BTE_S1_Y0              0xA5
#define RA8876_REG_BTE_S1_Y1              0xA6

#define RA8876_REG_BTE_DEST_STR0          0xA7
#define RA8876_REG_BTE_DEST_STR1          0xA8
#define RA8876_REG_BTE_DEST_STR2          0xA9
#define RA8876_REG_BTE_DEST_STR3          0xAA
#define RA8876_REG_BTE_DEST_WTH0          0xAB
#define RA8876_REG_BTE_DEST_WTH1          0xAC
#define RA8876_REG_BTE_DEST_X0            0xAD
#define RA8876_REG_BTE_DEST_X1            0xAE
#define RA8876_REG_BTE_DEST_Y0            0xAF
#define RA8876_REG_BTE_DEST_Y1            0xB0

#define RA8876_REG_BTE_WTH0               0xB1
#define RA8876_REG_BTE_WTH1               0xB2
#define RA8876_REG_BTE_HIG0               0xB3
#define RA8876_REG_BTE_HIG1               0xB4
#define RA8876_REG_APB_CTRL               0xB5

// Data sheet 19.9: Serial flash & SPI master control registers
#define RA8876_REG_DMA_CTRL               0xB6

#define RA8876_REG_DMA_START                (1 << 0)

#define RA8876_REG_SFL_CTRL               0xB7  // serial flash/ROM control register
#define RA8876_REG_SPI_DIVSOR             0xBB  // SPI clock period

                                       // 0xBC
                                       // ..
                                       // 0xCB  // undefined/reserved

// Data sheet 19.10: Text engine
#define RA8876_REG_CCR0                   0xCC  // character control register 0
#define RA8876_REG_CCR1                   0xCD  // character control register 1

#define RA8876_REG_CCR1_XSCALE_MASK       (3 << 2)
#define RA8876_REG_CCR1_XSCALE(s)           ((s) << 2)
#define RA8876_REG_CCR1_YSCALE_MASK       (3 << 0)
#define RA8876_REG_CCR1_YSCALE(s)           ((s) << 0)

#define RA8876_REG_GTFNT_SEL              0xCE  // genitop character ROM select
#define RA8876_REG_GTFNT_CR               0xCF  // genitop character ROM control register

#define RA8876_REG_FLDR                   0xD0  // character line gap register
#define RA8876_REG_F2FSSR                 0xD1  // character to character space setting register
#define RA8876_REG_FGCR                   0xD2  // foreground colour register - red
#define RA8876_REG_FGCG                   0xD3  // foreground colour register - green
#define RA8876_REG_FGCB                   0xD4  // foreground colour register - blue

                                       // 0xD4
                                       // ..
                                       // 0xDF  // undefined/reserved

// Data sheet 19.12: buffer RAM control registers
#define RA8876_REG_BFRAR                  0xE0  // buffer RAM attribute register
#define RA8876_REG_BFRMD                  0xE1  // buffer RAM mode & extended mode register
#define RA8876_REG_BFR_REF_ITVL0          0xE2  // buffer RAM auto refresh interval 0
#define RA8876_REG_BFR_REF_ITVL1          0xE3  // buffer RAM auto refresh interval 1
#define RA8876_REG_BFRCR                  0xE4  // buffer RAM control register

  #define RA8876_REG_BFRCR_INIT             (1 << 0) // start buffer ram initialization


                                       // 0xE5
                                       // ..
                                       // 0xFF  // undefined/reserved

class RA8876 : public Print
{
private:
  int                m_csPin;
  int                m_resetPin;

  int                m_width;
  int                m_height;
  uint8_t            m_depth;
  uint8_t            m_bpp;

  uint32_t           m_oscClock;       // OSC clock (external crystal) frequency in kHz

  RA8876PllParams    m_memPll;        // MCLK (memory) PLL parameters
  RA8876PllParams    m_corePll;       // CCLK (core) PLL parameters
  RA8876PllParams    m_scanPll;       // SCLK (LCD panel scan) PLL parameters

  SPISettings        m_spiSettings;
  RA8876RamInfo     *m_ramInfo;
  RA8876DisplayInfo *m_displayInfo;

  Color              m_textColor;
  int                m_textScaleX;
  int                m_textScaleY;

  RA8876FontSize     m_fontSize;
  RA8876FontFlags    m_fontFlags;

  // SPI
  void               _spiCmdWrite(uint8_t x);
  void               _spiDatWrite(uint8_t x);
  uint8_t            _spiDatRead();
  uint8_t            _spiSTSRead();

  // register access
  void               regWrite(uint8_t reg, uint8_t x);
  void               regWrite16(uint8_t reg, uint16_t x);
  void               regWrite32(uint8_t reg, uint32_t x);
  uint8_t            regRead(uint8_t reg);

  // reset functionality
  void               hardReset();
  void               softReset();

  // status waitUntilxxxx
  bool               waitUntilEmptyFifoWrite();
  bool               waitUntilEmptyFifoRead();
  bool               waitUntilFullFifoWrite();
  bool               waitUntilFullFifoRead();
  bool               waitUntilModeNormal();
  bool               waitUntilMemoryReady();
  bool               waitUntilStatusIdle();

  // initialization
  bool               calcPllParams(uint32_t targetFreq, int kMax, RA8876PllParams *pll);
  bool               calcClocks();
  bool               initPLL();
  bool               initMemory(RA8876RamInfo *info);
  bool               initDisplay();

  // chip mode
  void               setTextMode();
  void               setGraphicsMode();

  // font
  uint8_t            getFontEncoding(RA8876FontEncoding enc);

  // low-level drawing operations
  void               drawTwoPointShape(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, Color color, uint8_t reg, uint8_t cmd);                             // drawLine, drawRect, fillRect
  void               drawThreePointShape(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, Color color, uint8_t reg, uint8_t cmd); // drawTriangle, fillTriangle
  void               drawEllipseShape(uint16_t x, uint16_t y, uint16_t xrad, uint16_t yrad, Color color, uint8_t reg, uint8_t cmd);                            // drawCircle, fillCircle

public:
  RA8876(int csPin, int resetPin = 0);

  // initialization
  bool               init();
  void               colorBarTest(bool enabled);

  // display information
  uint16_t           getDisplayWidth();
  uint16_t           getDisplayHeight();
  
  // drawing
  void               clearScreen(Color color);
  void               putPixel(uint16_t x, uint16_t y, Color color);
  void               putPixels(uint16_t x, uint16_t y, Color *color, size_t cnt);
  void               drawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, Color color);
  void               drawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, Color color);
  void               fillRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, Color color);
  void               drawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, Color color);
  void               fillTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, Color color);
  void               drawCircle(uint16_t x, uint16_t y, uint16_t radius, Color color);
  void               fillCircle(uint16_t x, uint16_t y, uint16_t radius, Color color);
  void               drawEllipse(uint16_t x, uint16_t y, uint16_t x_radius, uint16_t y_radius, Color color);
  void               fillEllipse(uint16_t x, uint16_t y, uint16_t x_radius, uint16_t y_radius, Color color);

  // bte engine
  void               bteMemoryCopy(uint16_t sx, uint16_t sy, uint16_t dx, uint16_t dy, uint16_t w, uint16_t h);

  // font
  void               setFont(RA8876FontSize sz, RA8876FontEncoding enc = RA8876_FONT_ENCODING_8859_1);
  void               setTextCursor(uint16_t x, uint16_t y);
  uint16_t           getTextCursorX();
  uint16_t           getTextCursorY();
  uint16_t           getTextWidth();
  uint16_t           getTextHeight();
  void               setTextColor(Color color);
  void               setTextScale(int scale);
  void               setTextScale(int xScale, int yScale);
  void               putChar(char c);
  void               putChars(const char *buf, size_t sz);
  void               putChar16(uint16_t c);
  void               putChars16(const uint16_t *buf, size_t sz);

  // internal for print class
  virtual size_t     write(uint8_t c);
  virtual size_t     write(const uint8_t *buf, size_t sz);
};

//--------------------------------------------------------------------------
