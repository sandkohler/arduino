//--------------------------------------------------------------------------
// Copyright 2024, RIoT Secure AB
//
// @author Aaron Ardiri
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// RA8876 :: IMPLEMENTATION
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// constructor and intialization

RA8876::RA8876(int csPin, int resetPin)
{
  m_csPin       = csPin;
  m_resetPin    = resetPin;

  m_width       = 0;
  m_height      = 0;
  m_depth       = 0;
  m_bpp         = 0;

  m_oscClock    = 11000;  // Suggested OSC frequency is 11.0592MHz
  m_ramInfo     = &defaultRamInfo;
  m_displayInfo = &defaultDisplayInfo;
  m_textColor   = Color::White;
}

//--------------------------------------------------------------------------
// SPI interface

void RA8876::_spiCmdWrite(uint8_t x)
{
  digitalWrite(m_csPin, LOW);
  SPI.transfer(RA8876_CMD_WRITE);
  SPI.transfer(x);
  digitalWrite(m_csPin, HIGH);
}

void RA8876::_spiDatWrite(uint8_t x)
{
  digitalWrite(m_csPin, LOW);
  SPI.transfer(RA8876_DATA_WRITE);
  SPI.transfer(x);
  digitalWrite(m_csPin, HIGH);
}

uint8_t RA8876::_spiDatRead()
{
  uint8_t x;

  digitalWrite(m_csPin, LOW);
  SPI.transfer(RA8876_DATA_READ);
  x = SPI.transfer(0xff);
  digitalWrite(m_csPin, HIGH);
  return x;
}

uint8_t RA8876::_spiSTSRead()
{
  uint8_t x;

  digitalWrite(m_csPin, LOW);
  SPI.transfer(RA8876_STATUS_READ);
  x = SPI.transfer(0);
  digitalWrite(m_csPin, HIGH);
  return x;
}

void RA8876::regWrite(uint8_t reg, uint8_t v)
{
  _spiCmdWrite(reg);
  _spiDatWrite(v);
}

void RA8876::regWrite16(uint8_t reg, uint16_t v)
{
  // write two consecutive registers
  _spiCmdWrite(reg);
  _spiDatWrite( v       & 0xff);
  _spiCmdWrite(reg + 1);
  _spiDatWrite((v >> 8) & 0xff);
}

void RA8876::regWrite32(uint8_t reg, uint32_t v)
{
  // write four consecutive registers
  _spiCmdWrite(reg);
  _spiDatWrite( v        & 0xff);
  _spiCmdWrite(reg + 1);
  _spiDatWrite((v  >> 8) & 0xff);
  _spiCmdWrite(reg + 2);
  _spiDatWrite((v >> 16) & 0xff);
  _spiCmdWrite(reg + 3);
  _spiDatWrite((v >> 24) & 0xff);
}

uint8_t 
RA8876::regRead(uint8_t reg)
{
  _spiCmdWrite(reg);
  return _spiDatRead();
}

//--------------------------------------------------------------------------
// reset functionality

void
RA8876::hardReset()
{
  delay(5);
  digitalWrite(m_resetPin, LOW);
  delay(5);
  digitalWrite(m_resetPin, HIGH);
  delay(5);
}

void
RA8876::softReset()
{
  SPI.beginTransaction(m_spiSettings);

  // trigger soft reset
  regWrite(RA8876_REG_SRR, RA8876_REG_SRR_RESET);
  delay(5);

  // wait for normal operation
  waitUntilModeNormal();

  SPI.endTransaction();
}

//--------------------------------------------------------------------------
// status waitUntilxxxx

bool
RA8876::waitUntilModeNormal()
{ 
  bool ok = false;
  for (size_t i=0; i < 250; i++)
  {
    delay(1);
    ok = ((_spiSTSRead() & RA8876_STATUS_MODE_MASK) == RA8876_STATUS_MODE_NORM);
    if (ok) break;
  }
  return ok;
}

bool
RA8876::waitUntilMemoryReady()
{
  bool ok = false;

  for (size_t i=0; i < 250; i++)
  {
    delay(1);
    ok = ((_spiSTSRead() & RA8876_STATUS_BRAM_MASK) == RA8876_STATUS_BRAM_READY);
    if (ok) break;
  }

  return ok;
}

bool
RA8876::waitUntilStatusIdle()
{
  bool ok = false;

  for (size_t i=0; i < 250; i++)
  {
    delay(1);
    ok = ((_spiSTSRead() & RA8876_STATUS_TASK_MASK) == RA8876_STATUS_TASK_IDLE);
    if (ok) break;
  }

  return ok;
}

bool
RA8876::waitUntilEmptyFifoRead()
{
  bool ok = false;

  for (size_t i=0; i < 250; i++)
  {
    delay(1);
    ok = ((_spiSTSRead() & RA8876_STATUS_HMRFE_MASK) == RA8876_STATUS_HMRFE_E);
    if (ok) break;
  }

  return ok;
}

bool
RA8876::waitUntilEmptyFifoWrite()
{
  bool ok = false;

  for (size_t i=0; i < 250; i++)
  {
    delay(1);
    ok = ((_spiSTSRead() & RA8876_STATUS_HMWFE_MASK) == RA8876_STATUS_HMWFE_E);
    if (ok) break;
  }

  return ok;
}

bool
RA8876::waitUntilFullFifoRead()
{
  bool ok = false;

  for (size_t i=0; i < 250; i++)
  {
    delay(1);
    ok = ((_spiSTSRead() & RA8876_STATUS_HMRFF_MASK) == RA8876_STATUS_HMRFF_F);
    if (ok) break;
  }

  return ok;
}

bool
RA8876::waitUntilFullFifoWrite()
{
  bool ok = false;

  for (size_t i=0; i < 250; i++)
  {
    delay(1);
    ok = ((_spiSTSRead() & RA8876_STATUS_HMWFF_MASK) == RA8876_STATUS_HMWFF_F);
    if (ok) break;
  }

  return ok;
}

//--------------------------------------------------------------------------
// initialization

bool
RA8876::calcPllParams(uint32_t targetFreq, int kMax, RA8876PllParams *pll)
{
  bool     found;
  int      foundk, foundn;
  int      testk,  testn;
  uint32_t fvco, freq, error;
  uint32_t foundFreq;
  uint32_t foundError;  // amount lower than requested frequency

  // k of 0 (i.e. 2 ** 0 = 1) is possible, but not sure if it's a good idea.
  found = false;
  for (testk = 1; testk <= kMax; testk++)
  {
    if (m_oscClock % (1 << testk))   continue; // step size with this k would be fractional

    testn = (targetFreq / (m_oscClock / (1 << testk))) - 1;
    if ((testn < 1) || (testn > 63)) continue; // param n out of range for this k

    // fvco constraint found in data sheet section 6.1.2
    fvco = m_oscClock * (testn + 1);
    if ((fvco < 250000) && (fvco > 500000)) continue;  // fvco out of range

    // found some usable params, but perhaps at a lower frequency than requested.
    freq = (m_oscClock * (testn + 1)) / (1 << testk);
    error = targetFreq - freq;
    if ((!found) || (found && (foundError > error)))
    {
      found      = true;
      foundk     = testk;
      foundn     = testn;

      // keep for further potential refinement
      foundFreq  = freq;
      foundError = error;

      // no need to keep searching if the frequency match was exact
      if (foundError == 0) break;
    }
  }

  if (found)
  {
    pll->freq = foundFreq;
    pll->k    = foundk;
    pll->n    = foundn;
    pll->m    = 0; // this is always 0
  }

  return found;
}

bool
RA8876::calcClocks()
{
  bool     ok;
  uint32_t memClock;
  uint32_t coreClock;
  uint32_t scanClock;

  // default return value
  ok = false;

  // Data sheet section 2.5 gives max clocks:
  //  memClock : 166 MHz
  //  coreClock: 120 MHz (133MHz if not using internal font)
  //  scanClock: 100 MHz

  // mem clock target is the same as buffer ram speed, but capped at 166 MHz
  memClock = m_ramInfo->freq;
  if (memClock > 166000) memClock = 166000;
  if (!calcPllParams(memClock, 3, &m_memPll)) goto ra8876_calcClocks_done;

  // core clock target will be the same as the mem clock, but capped to 120mHz
  coreClock = m_memPll.freq;
  if (coreClock > 120000) coreClock = 120000;
  if (!calcPllParams(coreClock, 3, &m_corePll)) goto ra8876_calcClocks_done;

  // scan clock target will be the display's dot clock, but capped at 100 MHz
  scanClock = m_displayInfo->dotClock;
  if (scanClock > 100000) scanClock = 100000;
  if (!calcPllParams(scanClock, 7, &m_scanPll)) goto ra8876_calcClocks_done;

#if 0

  // display the clocks
  dbgUART.println(F("\nMem\n---"));
  dbgUART.print(F("Requested kHz: ")); dbgUART.println(m_ramInfo->freq);
  dbgUART.print(F("Actual kHz   : ")); dbgUART.println(m_memPll.freq);
  dbgUART.print(F("PLL k        : ")); dbgUART.println(m_memPll.k);
  dbgUART.print(F("PLL n        : ")); dbgUART.println(m_memPll.n);

  dbgUART.println(F("\nCore\n----"));
  dbgUART.print(F("kHz          : ")); dbgUART.println(m_corePll.freq);
  dbgUART.print(F("PLL k        : ")); dbgUART.println(m_corePll.k);
  dbgUART.print(F("PLL n        : ")); dbgUART.println(m_corePll.n);

  dbgUART.println(F("\nScan\n----"));
  dbgUART.print(F("Requested kHz: ")); dbgUART.println(m_displayInfo->dotClock);
  dbgUART.print(F("Actual kHz   : ")); dbgUART.println(m_scanPll.freq);
  dbgUART.print(F("PLL k        : ")); dbgUART.println(m_scanPll.k);
  dbgUART.print(F("PLL n        : ")); dbgUART.println(m_scanPll.n);
  dbgUART.println();

#endif

  // Data sheet section 6.1.1 rules:
  // 1. Core clock must be less than or equal to mem clock
  // 2. Core clock must be greater than half mem clock
  // 3. Core clock must be greater than (scan clock * 1.5)
  if  (m_corePll.freq      >   m_memPll.freq) goto ra8876_calcClocks_done;
  if ((m_corePll.freq * 2) <=  m_memPll.freq) goto ra8876_calcClocks_done;
  if  (m_corePll.freq      <= (m_scanPll.freq + (m_scanPll.freq >> 1))) goto ra8876_calcClocks_done;

  // got here? everything worked
  ok = true;

ra8876_calcClocks_done:

  return ok;
}

bool 
RA8876::initPLL()
{
  bool    ok;
  uint8_t reg;

  // default return value
  ok = false;

  SPI.beginTransaction(m_spiSettings);

  // configure PLL registers based on values
  regWrite(RA8876_REG_MPLLC1, m_memPll.k << 1  | m_memPll.m);
  regWrite(RA8876_REG_MPLLC2, m_memPll.n);

  regWrite(RA8876_REG_SPLLC1, m_corePll.k << 1 | m_corePll.m);
  regWrite(RA8876_REG_SPLLC2, m_corePll.n);

  regWrite(RA8876_REG_PPLLC1, m_scanPll.k << 1 | m_scanPll.m);
  regWrite(RA8876_REG_PPLLC2, m_scanPll.n);

  // trigger a reconfiguration of the PLLs (toggle bit)
  reg = regRead(RA8876_REG_CCR);
  reg &= ~RA8876_REG_CCR_CONF_PLL;
  regWrite(RA8876_REG_CCR, reg);
  delay(2);
  reg |=  RA8876_REG_CCR_CONF_PLL;
  regWrite(RA8876_REG_CCR, reg);
  delay(2);

  // check the changes were made successfully
  ok = ((regRead(RA8876_REG_CCR) & RA8876_REG_CCR_CONF_PLL) == RA8876_REG_CCR_CONF_PLL);

  SPI.endTransaction();

  return ok;
}

bool
RA8876::initMemory(RA8876RamInfo *info)
{
  bool ok;

  // default return value
  ok = false;

  SPI.beginTransaction(m_spiSettings);

  // Data sheet 19.12: buffer RAM control registers

  // configure RAM attributes and latency
  regWrite(RA8876_REG_BFRAR, 0x28);         // must be b(000101000)
  regWrite(RA8876_REG_BFRMD, info->latency);

  // configure auto refresh intervals
  regWrite(RA8876_REG_BFR_REF_ITVL0, 0x00); // disable auto refresh
  regWrite(RA8876_REG_BFR_REF_ITVL1, 0x00); // disable auto refresh

  // trigger buffer RAM initialization
  regWrite(RA8876_REG_BFRCR, RA8876_REG_BFRCR_INIT);

  // wait for buffer RAM to be ready
  ok = waitUntilMemoryReady();

  SPI.endTransaction();

  return ok;
}

bool
RA8876::initDisplay()
{
  bool     ok;
  uint8_t  reg;
  uint16_t height;

  // default return value
  ok = false;

  SPI.beginTransaction(m_spiSettings);

  //
  // setup the video output stream
  //

  // set chip config register to 24-bit output
  reg = regRead(RA8876_REG_CCR);
  reg = (reg & ~RA8876_REG_CCR_TFT_MASK)       | RA8876_REG_CCR_TFT_24BIT;
  regWrite(RA8876_REG_CCR, reg);

  // configure memory control register
  reg = regRead(RA8876_REG_MACR);
  reg = (reg & ~RA8876_REG_MACR_MPU_MASK)      | RA8876_REG_MACR_MPU_DIRECT;   // direct write
  reg = (reg & ~RA8876_REG_MACR_DIRR_MASK)     | RA8876_REG_MACR_DIRR_LRTB;    // left-right-top-bottom
  reg = (reg & ~RA8876_REG_MACR_DIRW_MASK)     | RA8876_REG_MACR_DIRW_LRTB;    // left-right-top-bottom
  regWrite(RA8876_REG_MACR, reg);

  // configure input control register
  reg = regRead(RA8876_REG_ICR);
  reg = (reg & ~RA8876_REG_ICR_MODE_MASK)      | RA8876_REG_ICR_MODE_GRAPHIC;  // graphic mode
  reg = (reg & ~RA8876_REG_ICR_MEM_MASK)       | RA8876_REG_ICR_MEM_BRAM;      // buffer ram
  regWrite(RA8876_REG_ICR, reg); 

  // configure display configuration register
  reg = regRead(RA8876_REG_DPCR);
  reg = (reg & ~RA8876_REG_DPCR_PCLK_MASK)     | RA8876_REG_DPCR_PCLK_RISE;    // fetch on rising
  reg = (reg & ~RA8876_REG_DPCR_DISPLAY_MASK)  | RA8876_REG_DPCR_DISPLAY_OFF;  // display on
  reg = (reg & ~RA8876_REG_DPCR_TEST_MASK)     | RA8876_REG_DPCR_TEST_DISABLE; // test disable
  reg = (reg & ~RA8876_REG_DPCR_VDIR_MASK)     | RA8876_REG_DPCR_VDIR_TB;      // scan top-bottom
  reg = (reg & ~RA8876_REG_DPCR_XPDA_MASK)     | RA8876_REG_DPCR_XPDA_RGB;     // RGB output
  regWrite(RA8876_REG_DPCR, reg);

  // configure panel scan clock and data settings
  reg = regRead(RA8876_REG_PCSR);
  if (m_displayInfo->hSyncPolarity == 1) reg = (reg & ~RA8876_REG_PCSR_XHSYNC_MASK)  | RA8876_REG_PCSR_XHSYNC_HIGH;  // HSYNC +
  else                                   reg = (reg & ~RA8876_REG_PCSR_XHSYNC_MASK)  | RA8876_REG_PCSR_XHSYNC_LOW;   // HSYNC -
  if (m_displayInfo->vSyncPolarity == 1) reg = (reg & ~RA8876_REG_PCSR_XVSYNC_MASK)  | RA8876_REG_PCSR_XVSYNC_HIGH;  // VSYNC +
  else                                   reg = (reg & ~RA8876_REG_PCSR_XVSYNC_MASK)  | RA8876_REG_PCSR_XVSYNC_LOW;   // VSYNC -
  reg = (reg & ~RA8876_REG_PCSR_XDE_MASK)      | RA8876_REG_PCSR_XDE_HIGH;     // XDE +
  reg = (reg & ~RA8876_REG_PCSR_XDEI_MASK)     | RA8876_REG_PCSR_XDEI_LOW;
  reg = (reg & ~RA8876_REG_PCSR_XPCLKI_MASK)   | RA8876_REG_PCSR_XPCLKI_LOW;
  reg = (reg & ~RA8876_REG_PCSR_XPDATI_MASK)   | RA8876_REG_PCSR_XPDATI_LOW;
  if (m_displayInfo->hSyncPolarity == 1) reg = (reg & ~RA8876_REG_PCSR_XHSYNCI_MASK) | RA8876_REG_PCSR_XHSYNCI_HIGH; // HSYNC +
  else                                   reg = (reg & ~RA8876_REG_PCSR_XHSYNCI_MASK) | RA8876_REG_PCSR_XHSYNCI_LOW;  // HSYNC -
  if (m_displayInfo->vSyncPolarity == 1) reg = (reg & ~RA8876_REG_PCSR_XVSYNCI_MASK) | RA8876_REG_PCSR_XVSYNCI_HIGH; // VSYNC +
  else                                   reg = (reg & ~RA8876_REG_PCSR_XVSYNCI_MASK) | RA8876_REG_PCSR_XVSYNCI_LOW;  // VSYNC -
  regWrite(RA8876_REG_PCSR, reg);

  // set display width
  regWrite(RA8876_REG_HDWR,         ((m_displayInfo->width / 8) - 1)            & 0xff);
  regWrite(RA8876_REG_HDWFTR,        (m_displayInfo->width % 8)                 & 0x07);

  // set display height
  regWrite(RA8876_REG_VDHR0,         (m_displayInfo->height - 1)                & 0xff);
  regWrite(RA8876_REG_VDHR1,        ((m_displayInfo->height - 1) >> 8)          & 0x07);

  // set horizontal non-display (back porch)
  regWrite(RA8876_REG_HNDR,         ((m_displayInfo->hBackPorch / 8) - 1)       & 0x1f);
  regWrite(RA8876_REG_HNDFTR,        (m_displayInfo->hBackPorch % 8)            & 0x0f);

  // set horizontal start position (front porch)
  regWrite(RA8876_REG_HSTR,        (((m_displayInfo->hFrontPorch + 4) / 8) - 1) & 0x1f);

  // set HSYNC pulse width
  regWrite(RA8876_REG_HPWR,        (((m_displayInfo->hPulseWidth + 4) / 8) - 1) & 0x1f);

  // set vertical non-display (back porch)
  regWrite(RA8876_REG_VNDR0,        (m_displayInfo->vBackPorch - 1)             & 0xff);
  regWrite(RA8876_REG_VNDR1,       ((m_displayInfo->vBackPorch - 1) >> 8)       & 0x03);

  // set vertical start position (front porch)
  regWrite(RA8876_REG_VSTR,         (m_displayInfo->vFrontPorch - 1)            & 0xff);

  // set VSYNC pulse width
  regWrite(RA8876_REG_VPWR,         (m_displayInfo->vPulseWidth - 1)            & 0x3f);

  //
  // setup the window and active canvas
  //

  // configure window control register
  reg = regRead(RA8876_REG_MPWCTR);
  reg = (reg & ~RA8876_REG_MPWCTR_PIP1_MASK)   | RA8876_REG_MPWCTR_PIP1_DISABLE; // disable PIP1
  reg = (reg & ~RA8876_REG_MPWCTR_PIP2_MASK)   | RA8876_REG_MPWCTR_PIP2_DISABLE; // disable PIP2
       if (m_depth ==  8) reg = (reg & ~RA8876_REG_MPWCTR_DEPTH_MASK) | RA8876_REG_MPWCTR_DEPTH_8BPP;  // 8bpp
  else if (m_depth == 16) reg = (reg & ~RA8876_REG_MPWCTR_DEPTH_MASK) | RA8876_REG_MPWCTR_DEPTH_16BPP; // 16bpp
  else if (m_depth == 24) reg = (reg & ~RA8876_REG_MPWCTR_DEPTH_MASK) | RA8876_REG_MPWCTR_DEPTH_24BPP; // 24bpp
  reg = (reg & ~RA8876_REG_MPWCTR_SYNC_MASK)   | RA8876_REG_MPWCTR_SYNC_ENABLE; // sync enable
  regWrite(RA8876_REG_MPWCTR, reg);

  // set main window start address to 0
  regWrite(RA8876_REG_MISA0,         0);
  regWrite(RA8876_REG_MISA1,         0);
  regWrite(RA8876_REG_MISA2,         0);
  regWrite(RA8876_REG_MISA3,         0);

  // set main window image width
  regWrite(RA8876_REG_MIW0,          m_width       & 0xff);
  regWrite(RA8876_REG_MIW1,         (m_width >> 8) & 0xff);

  // set main window start coordinates
  regWrite(RA8876_REG_MWULX0,        0);
  regWrite(RA8876_REG_MWULX1,        0);
  regWrite(RA8876_REG_MWULY0,        0);
  regWrite(RA8876_REG_MWULY1,        0);
 
  // set canvas start address
  regWrite(RA8876_REG_CVSSA0,        0);
  regWrite(RA8876_REG_CVSSA1,        0);
  regWrite(RA8876_REG_CVSSA2,        0);
  regWrite(RA8876_REG_CVSSA3,        0);

  // set canvas width
  regWrite(RA8876_REG_CVS_IMWTH0,    m_width       & 0xff);
  regWrite(RA8876_REG_CVS_IMWTH1,   (m_width >> 8) & 0xff);

  // set active window start coordinates
  regWrite(RA8876_REG_AWUL_X0,       0);
  regWrite(RA8876_REG_AWUL_X1,       0);
  regWrite(RA8876_REG_AWUL_Y0,       0);
  regWrite(RA8876_REG_AWUL_Y1,       0);

  // set active window dimensions - this is logical height, not display height
  height = (m_ramInfo->sz / ((uint32_t)m_width * (m_depth >> 3)));
  if (height > 4095) height = 4095; // this is the maximum height as per document
  regWrite(RA8876_REG_AW_WTH0,        m_width        & 0xff);
  regWrite(RA8876_REG_AW_WTH1,       (m_width  >> 8) & 0xff);
  regWrite(RA8876_REG_AW_HT0,           height       & 0xff);
  regWrite(RA8876_REG_AW_HT1,        (  height >> 8) & 0xff); // this is the "logical" height (based on RAM)

#if 0
  dbgUART.print(F("BRAM width  = "));
  dbgUART.println(m_width);
  dbgUART.print(F("BRAM height = "));
  dbgUART.println(height);
  dbgUART.println();
#endif

  // set canvas addressing mode/colour depth
  reg = regRead(RA8876_REG_AW_COLOR);
  reg = (reg & ~RA8876_REG_AW_COLOR_ADDR_MASK) | RA8876_REG_AW_COLOR_ADDR_BLOCK;
       if (m_depth ==  8) reg = (reg & ~RA8876_REG_AW_COLOR_DEPTH_MASK) | RA8876_REG_AW_COLOR_DEPTH_8BPP;  // 8bpp
  else if (m_depth == 16) reg = (reg & ~RA8876_REG_AW_COLOR_DEPTH_MASK) | RA8876_REG_AW_COLOR_DEPTH_16BPP; // 16bpp
  else if (m_depth == 24) reg = (reg & ~RA8876_REG_AW_COLOR_DEPTH_MASK) | RA8876_REG_AW_COLOR_DEPTH_24BPP; // 24bpp
  regWrite(RA8876_REG_AW_COLOR, reg);

  //
  // configure BTE engine foundations
  //

  // S0 start address, width and x,y position
  regWrite(RA8876_REG_BTE_S0_STR0,    0);
  regWrite(RA8876_REG_BTE_S0_STR1,    0);
  regWrite(RA8876_REG_BTE_S0_STR2,    0);
  regWrite(RA8876_REG_BTE_S0_STR3,    0);
  regWrite(RA8876_REG_BTE_S0_WTH0,    m_width        & 0xff);
  regWrite(RA8876_REG_BTE_S0_WTH1,   (m_width  >> 8) & 0xff);
  regWrite(RA8876_REG_BTE_S0_X0,      0);
  regWrite(RA8876_REG_BTE_S0_X1,      0);
  regWrite(RA8876_REG_BTE_S0_Y0,      0);
  regWrite(RA8876_REG_BTE_S0_Y1,      0);

  // S1 start address, width and x,y position
  regWrite(RA8876_REG_BTE_S1_STR0,    0);
  regWrite(RA8876_REG_BTE_S1_STR1,    0);
  regWrite(RA8876_REG_BTE_S1_STR2,    0);
  regWrite(RA8876_REG_BTE_S1_STR3,    0);
  regWrite(RA8876_REG_BTE_S1_WTH0,    m_width        & 0xff);
  regWrite(RA8876_REG_BTE_S1_WTH1,   (m_width  >> 8) & 0xff);
  regWrite(RA8876_REG_BTE_S1_X0,      0);
  regWrite(RA8876_REG_BTE_S1_X1,      0);
  regWrite(RA8876_REG_BTE_S1_Y0,      0);
  regWrite(RA8876_REG_BTE_S1_Y1,      0);

  // DST start address, width and x,y position
  regWrite(RA8876_REG_BTE_DEST_STR0,  0);
  regWrite(RA8876_REG_BTE_DEST_STR1,  0);
  regWrite(RA8876_REG_BTE_DEST_STR2,  0);
  regWrite(RA8876_REG_BTE_DEST_STR3,  0);
  regWrite(RA8876_REG_BTE_DEST_WTH0,  m_width        & 0xff);
  regWrite(RA8876_REG_BTE_DEST_WTH1, (m_width  >> 8) & 0xff);
  regWrite(RA8876_REG_BTE_DEST_X0,    0);
  regWrite(RA8876_REG_BTE_DEST_X1,    0);
  regWrite(RA8876_REG_BTE_DEST_Y0,    0);
  regWrite(RA8876_REG_BTE_DEST_Y1,    0);

  // S0, S0 and DST color depths
  reg = 0;
       if (m_depth ==  8) reg = RA8876_REG_BTE_S0_DEPTH_8BPP  | RA8876_REG_BTE_S1_DEPTH_8BPP  | RA8876_REG_BTE_DEST_DEPTH_8BPP;  // 8bpp
  else if (m_depth == 16) reg = RA8876_REG_BTE_S0_DEPTH_16BPP | RA8876_REG_BTE_S1_DEPTH_16BPP | RA8876_REG_BTE_DEST_DEPTH_16BPP; // 16bpp
  else if (m_depth == 24) reg = RA8876_REG_BTE_S0_DEPTH_24BPP | RA8876_REG_BTE_S1_DEPTH_24BPP | RA8876_REG_BTE_DEST_DEPTH_24BPP; // 24bpp
  regWrite(RA8876_REG_BTE_COLR, reg);

  //
  // initialization is complete
  //

  // turn on display
  reg = regRead(RA8876_REG_DPCR);
  reg = (reg & ~RA8876_REG_DPCR_DISPLAY_MASK)  | RA8876_REG_DPCR_DISPLAY_ON; // display on
  regWrite(RA8876_REG_DPCR, reg);

  SPI.endTransaction();

  // if we got here, all ok
  ok = true;

  return ok;
}

bool
RA8876::init()
{
  bool ok;

  // default return value
  ok = false;

  m_width  = m_displayInfo->width;
  m_height = m_displayInfo->height;
  m_depth  = RA8876_COLOR_DEPTH;
  m_bpp    = (m_depth << 3);       // 8bpp = 1, 16bpp = 2, 24bpp = 3

  // set up chip select pin
  pinMode(m_csPin, OUTPUT);
  digitalWrite(m_csPin, HIGH);

  // set up reset pin, if provided
  if (m_resetPin >= 0)
  {
    pinMode(m_resetPin, OUTPUT);
    digitalWrite(m_resetPin, HIGH);

    // perform a hard reset
    hardReset();
  }

  // calculate the clocks required
  if (!calcClocks()) goto ra8876_init_done;

  SPI.begin();
  m_spiSettings = SPISettings(RA8876_SPI_SPEED, MSBFIRST, SPI_MODE3);

  // SPI is now up, can do a soft reset if no hard reset was possible earlier
  if (m_resetPin < 0) softReset();

  // initialize PLL, memory and the display
  if (!initPLL())             goto ra8876_init_done;
  if (!initMemory(m_ramInfo)) goto ra8876_init_done;
  if (!initDisplay())         goto ra8876_init_done;

  // set default font
  setFont(RA8876_FONT_SIZE_16);
  setTextScale(1);

  // ensure the chip is in graphics mode
  setGraphicsMode();

  // if we got here, we are good
  ok = true;

ra8876_init_done:;

  return ok;
}

void
RA8876::colorBarTest(bool enabled)
{
  uint8_t reg;

  SPI.beginTransaction(m_spiSettings);

  // toggle the display test color bar bit as needed
  reg = regRead(RA8876_REG_DPCR);
  if (enabled) reg = (reg & ~RA8876_REG_DPCR_TEST_MASK) | RA8876_REG_DPCR_TEST_ENABLE;
  else         reg = (reg & ~RA8876_REG_DPCR_TEST_MASK) | RA8876_REG_DPCR_TEST_DISABLE;
  regWrite(RA8876_REG_DPCR, reg);

  SPI.endTransaction();
}

//--------------------------------------------------------------------------
// display information

uint16_t
RA8876::getDisplayWidth()
{ 
  return m_width;
};

uint16_t
RA8876::getDisplayHeight() 
{
  return m_height;
};

//--------------------------------------------------------------------------
// chip mode

void
RA8876::setTextMode()
{
  uint8_t reg;

  // restore text colour
  regWrite(RA8876_REG_FGCR, m_textColor.r);
  regWrite(RA8876_REG_FGCG, m_textColor.g);
  regWrite(RA8876_REG_FGCB, m_textColor.b);

  // wait for previous tasks to complete
  waitUntilStatusIdle();

  // set text mode
  reg = regRead(RA8876_REG_ICR);
  reg = (reg & ~RA8876_REG_ICR_MODE_MASK) | RA8876_REG_ICR_MODE_TEXT;
  regWrite(RA8876_REG_ICR, reg);
}

void
RA8876::setGraphicsMode()
{
  uint8_t reg;

  // wait for previous tasks to complete
  waitUntilStatusIdle();

  // set graphics mode
  reg = regRead(RA8876_REG_ICR);
  reg = (reg & ~RA8876_REG_ICR_MODE_MASK) | RA8876_REG_ICR_MODE_GRAPHIC;
  regWrite(RA8876_REG_ICR, reg);
}

//--------------------------------------------------------------------------
// drawing

void
RA8876::clearScreen(Color color) 
{ 
  setTextCursor(0, 0); 
  fillRectangle(0, 0, m_width, m_height, color);
};

void
RA8876::putPixel(uint16_t x, uint16_t y, Color color)
{
  SPI.beginTransaction(m_spiSettings);

  // set the location of the pixel
  regWrite(RA8876_REG_CURH0,  x       & 0xff);
  regWrite(RA8876_REG_CURH1, (x >> 8) & 0xff);
  regWrite(RA8876_REG_CURV0,  y       & 0xff);
  regWrite(RA8876_REG_CURV1, (y >> 8) & 0xff);

  // draw the pixel
  switch (m_depth)
  {
    case 24:
         {
           _spiCmdWrite(RA8876_REG_MRWDP);
           _spiDatWrite(color.b);
           _spiDatWrite(color.g);
           _spiDatWrite(color.r);
         }
         break;

    case 16:
         {
           uint16_t rgb565 = color.toRGB565();
           _spiCmdWrite(RA8876_REG_MRWDP);
           _spiDatWrite( rgb565       & 0xff);
           _spiDatWrite((rgb565 >> 8) & 0xff);
         }
         break;

    case 8:
         {
           uint8_t rgb332 = color.toRGB332();
          _spiCmdWrite(RA8876_REG_MRWDP);
          _spiDatWrite( rgb332       & 0xff);
         }
         break;
  }

  SPI.endTransaction();
}

void
RA8876::putPixels(uint16_t x, uint16_t y, Color *color, size_t cnt)
{
  SPI.beginTransaction(m_spiSettings);

  // set the location of the pixel
  regWrite(RA8876_REG_CURH0,  x       & 0xff);
  regWrite(RA8876_REG_CURH1, (x >> 8) & 0xff);
  regWrite(RA8876_REG_CURV0,  y       & 0xff);
  regWrite(RA8876_REG_CURV1, (y >> 8) & 0xff);

  // draw the pixel
  switch (m_depth)
  {
    case 24:
         {
           _spiCmdWrite(RA8876_REG_MRWDP);
           for (size_t i=0; i<cnt; i++)
           {
             _spiDatWrite(color[i].b);
             _spiDatWrite(color[i].g);
             _spiDatWrite(color[i].r);
           }
         }
         break;

    case 16:
         {
           _spiCmdWrite(RA8876_REG_MRWDP);
           for (size_t i=0; i<cnt; i++)
           {
             uint16_t rgb565 = color[i].toRGB565();
             _spiDatWrite( rgb565       & 0xff);
             _spiDatWrite((rgb565 >> 8) & 0xff);
           }
         }
         break;

    case 8:
         {
           _spiCmdWrite(RA8876_REG_MRWDP);
           for (size_t i=0; i<cnt; i++)
           {
             uint8_t rgb332 = color[i].toRGB332();
             _spiDatWrite( rgb332       & 0xff);
           }
         }
         break;
  }

  SPI.endTransaction();
}

void
RA8876::drawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, Color color)
{
  drawTwoPointShape(x1, y1, x2, y2, color, RA8876_REG_DCR0, RA8876_REG_DCR0_DRAW_LINE);
};

void
RA8876::drawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, Color color)
{ 
  drawTwoPointShape(x1, y1, x2, y2, color, RA8876_REG_DCR1, RA8876_REG_DCR1_DRAW_RECT);
};

void
RA8876::fillRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, Color color)
{
  drawTwoPointShape(x1, y1, x2, y2, color, RA8876_REG_DCR1, RA8876_REG_DCR1_DRAW_RECT_FILL);
};

void
RA8876::drawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, Color color)
{
  drawThreePointShape(x1, y1, x2, y2, x3, y3, color, RA8876_REG_DCR0, RA8876_REG_DCR0_DRAW_TRIANGLE);
};

void
RA8876::fillTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, Color color)
{
  drawThreePointShape(x1, y1, x2, y2, x3, y3, color, RA8876_REG_DCR0, RA8876_REG_DCR0_DRAW_TRIANGLE_FILL);
};

void
RA8876::drawCircle(uint16_t x, uint16_t y, uint16_t radius, Color color)
{
  drawEllipseShape(x, y, radius, radius, color, RA8876_REG_DCR1, RA8876_REG_DCR1_DRAW_CIRCLE);
};

void
RA8876::fillCircle(uint16_t x, uint16_t y, uint16_t radius, Color color)
{
  drawEllipseShape(x, y, radius, radius, color, RA8876_REG_DCR1, RA8876_REG_DCR1_DRAW_CIRCLE_FILL);
};

void
RA8876::drawEllipse(uint16_t x, uint16_t y, uint16_t x_radius, uint16_t y_radius, Color color)
{
  drawEllipseShape(x, y, x_radius, y_radius, color, RA8876_REG_DCR1, RA8876_REG_DCR1_DRAW_ELLIPSE);
};

void
RA8876::fillEllipse(uint16_t x, uint16_t y, uint16_t x_radius, uint16_t y_radius, Color color)
{
  drawEllipseShape(x, y, x_radius, y_radius, color, RA8876_REG_DCR1, RA8876_REG_DCR1_DRAW_ELLIPSE_FILL);
};

//--------------------------------------------------------------------------
// bte engine
void
RA8876::bteMemoryCopy(uint16_t sx, uint16_t sy, uint16_t dx, uint16_t dy, uint16_t w, uint16_t h)
{
  uint8_t reg;

  SPI.beginTransaction(m_spiSettings);

  // set S0 co-ordinates
  regWrite(RA8876_REG_BTE_S0_X0,    sx       & 0xff);
  regWrite(RA8876_REG_BTE_S0_X1,   (sx >> 8) & 0xff);
  regWrite(RA8876_REG_BTE_S0_Y0,    sy       & 0xff);
  regWrite(RA8876_REG_BTE_S0_Y1,   (sy >> 8) & 0xff);

  // set DEST co-ordinates
  regWrite(RA8876_REG_BTE_DEST_X0,  dx       & 0xff);
  regWrite(RA8876_REG_BTE_DEST_X1, (dx >> 8) & 0xff);
  regWrite(RA8876_REG_BTE_DEST_Y0,  dy       & 0xff);
  regWrite(RA8876_REG_BTE_DEST_Y1, (dy >> 8) & 0xff);

  // set the copy width and height
  regWrite(RA8876_REG_BTE_WTH0,     w       & 0xff);
  regWrite(RA8876_REG_BTE_WTH1,    (w >> 8) & 0xff);
  regWrite(RA8876_REG_BTE_HIG0,     h       & 0xff);
  regWrite(RA8876_REG_BTE_HIG1,    (h >> 8) & 0xff);

  // define the BTE mode to use - simply copy source to destination
  reg = RA8876_REG_BTE_ROP_CODE_12 | RA8876_REG_BTE_MEM_CPY_ROP; // copy with ROP where DEST = S0
  regWrite(RA8876_REG_BTE_CTRL1, reg);

  // start the operation
  reg = regRead(RA8876_REG_BTE_CTRL0);
  reg |= RA8876_REG_BTE_ENABLE;
  regWrite(RA8876_REG_BTE_CTRL0, reg);

  // wait for completion
  waitUntilStatusIdle();

  SPI.endTransaction();
}

//--------------------------------------------------------------------------
// font utils

uint8_t
RA8876::getFontEncoding(RA8876FontEncoding enc)
{
  uint8_t e;
  switch (enc)
  {
    case RA8876_FONT_ENCODING_8859_2: e = 0x01; break;
    case RA8876_FONT_ENCODING_8859_4: e = 0x02; break;
    case RA8876_FONT_ENCODING_8859_5: e = 0x03; break;
    default:                          e = 0x00; break; // ISO-8859-1
  }

  return e;
}

void 
RA8876::setFont(RA8876FontSize sz, RA8876FontEncoding enc)
{
  m_fontSize   = sz;
  m_fontFlags  = 0;

  SPI.beginTransaction(m_spiSettings);

  regWrite(RA8876_REG_CCR0, 0x00 | ((sz & 0x03) << 4) | getFontEncoding(enc));

  uint8_t ccr1 = regRead(RA8876_REG_CCR1);
  ccr1 |= 0x40;  // transparent background
  regWrite(RA8876_REG_CCR1, ccr1);

  SPI.endTransaction();
}

void 
RA8876::setTextCursor(uint16_t x, uint16_t y)
{
  SPI.beginTransaction(m_spiSettings);

  regWrite16(RA8876_REG_F_CURX0,  x       & 0xff);
  regWrite16(RA8876_REG_F_CURX1, (x >> 8) & 0xff);
  regWrite16(RA8876_REG_F_CURY0,  y       & 0xff);
  regWrite16(RA8876_REG_F_CURY1, (y >> 8) & 0xff);

  SPI.endTransaction();
}

uint16_t 
RA8876::getTextCursorX()
{
  uint16_t x;

  SPI.beginTransaction(m_spiSettings);
  x  = (uint16_t)regRead(RA8876_REG_F_CURX0);
  x |= (uint16_t)regRead(RA8876_REG_F_CURX1) << 8;
  SPI.endTransaction();

  return x;
}

uint16_t 
RA8876::getTextCursorY()
{
  uint16_t y;

  SPI.beginTransaction(m_spiSettings);
  y  = (uint16_t)regRead(RA8876_REG_F_CURY0);
  y |= (uint16_t)regRead(RA8876_REG_F_CURY1) << 8;
  SPI.endTransaction();

  return y;
}

uint16_t 
RA8876::getTextHeight()
{
  return ((m_fontSize + 2) * 8) * m_textScaleY;
}

uint16_t 
RA8876::getTextWidth()
{
  return ((m_fontSize + 2) * 4) * m_textScaleY;
}

void
RA8876::setTextColor(Color color) 
{ 
  m_textColor = color;
}

void
RA8876::setTextScale(int scale) 
{
  setTextScale(scale, scale);
}

void
RA8876::setTextScale(int xScale, int yScale)
{
  uint8_t reg;

  xScale = constrain(xScale, 1, 4); // keep within values 1, 2, 3 and 4
  yScale = constrain(yScale, 1, 4); // keep within values 1, 2, 3 and 4

  m_textScaleX = xScale;
  m_textScaleY = yScale;

  SPI.beginTransaction(m_spiSettings);

  reg = regRead(RA8876_REG_CCR1);
  reg = (reg & ~RA8876_REG_CCR1_XSCALE_MASK) | RA8876_REG_CCR1_XSCALE(xScale - 1);
  reg = (reg & ~RA8876_REG_CCR1_YSCALE_MASK) | RA8876_REG_CCR1_YSCALE(yScale - 1);
  regWrite(RA8876_REG_CCR1, reg);

  SPI.endTransaction();
}

void
RA8876::putChar(char c) 
{
  putChars(&c, 1);
}

void
RA8876::putChars(const char *buf, size_t sz)
{
  SPI.beginTransaction(m_spiSettings);

  setTextMode();

  // write characters
  _spiCmdWrite(RA8876_REG_MRWDP);
  for (size_t i = 0; i < sz; i++)
  {
    _spiDatWrite(buf[i]);
    waitUntilEmptyFifoWrite();
  }

  setGraphicsMode();

  SPI.endTransaction();
}

void
RA8876::putChar16(uint16_t c)
{
  putChars16(&c, 1);
}

void
RA8876::putChars16(const uint16_t *buf, size_t sz)
{
  SPI.beginTransaction(m_spiSettings);

  // this is a text mode operation
  setTextMode();

  // write characters
  _spiCmdWrite(RA8876_REG_MRWDP);
  for (size_t i = 0; i < sz; i++)
  {
    _spiDatWrite((buf[i] >> 8) & 0xff);
    waitUntilEmptyFifoWrite();
    _spiDatWrite( buf[i]       & 0xff);
    waitUntilEmptyFifoWrite();
  }

  // revert back to graphics mode
  setGraphicsMode();

  SPI.endTransaction();
}

size_t
RA8876::write(uint8_t c)
{
  return write(&c, 1);
}

size_t 
RA8876::write(const uint8_t *buf, size_t sz)
{
  SPI.beginTransaction(m_spiSettings);

  // this is a text mode operation
  setTextMode();

  // set current register for writing to memory
  _spiCmdWrite(RA8876_REG_MRWDP);
  for (size_t i = 0; i < sz; i++)
  {
    uint8_t c = buf[i];

    if (c == '\r'); // ignored
    else
    if (c == '\n')
    {
      setTextCursor(0, getTextCursorY() + getTextHeight());
      _spiCmdWrite(RA8876_REG_MRWDP);  // reset current register for writing to memory
    }
    else
    if ((m_fontFlags & RA8876_FONT_FLAG_XLAT_FULLWIDTH) && ((c >= 0x21) || (c <= 0x7F)))
    {
      // translate ASCII to Unicode fullwidth form (for Chinese fonts that lack ASCII)
      uint16_t fwc = c - 0x21 + 0xFF01;

      _spiDatWrite((fwc >> 8) & 0xff);
      waitUntilEmptyFifoWrite();
      _spiDatWrite( fwc       & 0xff);
      waitUntilEmptyFifoWrite();
    }
    else
    {
      _spiDatWrite(c);
      waitUntilEmptyFifoWrite();
    }
  }

  // revert back to graphics mode
  setGraphicsMode();

  SPI.endTransaction();

  return sz;
}

//--------------------------------------------------------------------------
// low-level drawing operations

void
RA8876::drawTwoPointShape(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, Color color, uint8_t reg, uint8_t cmd)
{
  SPI.beginTransaction(m_spiSettings);

  // first point
  regWrite(RA8876_REG_DLHSR0,  x1       & 0xff);
  regWrite(RA8876_REG_DLHSR1, (x1 >> 8) & 0xff);
  regWrite(RA8876_REG_DLVSR0,  y1       & 0xff);
  regWrite(RA8876_REG_DLVSR1, (y1 >> 8) & 0xff);

  // second point
  regWrite(RA8876_REG_DLHER0,  x2       & 0xff);
  regWrite(RA8876_REG_DLHER1, (x2 >> 8) & 0xff);
  regWrite(RA8876_REG_DLVER0,  y2       & 0xff);
  regWrite(RA8876_REG_DLVER1, (y2 >> 8) & 0xff);

  // colour
  regWrite(RA8876_REG_FGCR,    color.r);
  regWrite(RA8876_REG_FGCG,    color.g);
  regWrite(RA8876_REG_FGCB,    color.b);

  // draw
  regWrite(reg, cmd);

  // wait for completion
  waitUntilStatusIdle();

  SPI.endTransaction();
}

void
RA8876::drawThreePointShape(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, Color color, uint8_t reg, uint8_t cmd)
{
  SPI.beginTransaction(m_spiSettings);

  // first point
  regWrite(RA8876_REG_DLHSR0,  x1       & 0xff);
  regWrite(RA8876_REG_DLHSR1, (x1 >> 8) & 0xff);
  regWrite(RA8876_REG_DLVSR0,  y1       & 0xff);
  regWrite(RA8876_REG_DLVSR1, (y1 >> 8) & 0xff);

  // second point
  regWrite(RA8876_REG_DLHER0,  x2       & 0xff);
  regWrite(RA8876_REG_DLHER1, (x2 >> 8) & 0xff);
  regWrite(RA8876_REG_DLVER0,  y2       & 0xff);
  regWrite(RA8876_REG_DLVER1, (y2 >> 8) & 0xff);

  // third point
  regWrite(RA8876_REG_DTPH0,   x3       & 0xff);
  regWrite(RA8876_REG_DTPH1,  (x3 >> 8) & 0xff);
  regWrite(RA8876_REG_DTPV0,   y3       & 0xff);
  regWrite(RA8876_REG_DTPV1,  (y3 >> 8) & 0xff);

  // colour
  regWrite(RA8876_REG_FGCR,   color.r);
  regWrite(RA8876_REG_FGCG,   color.g);
  regWrite(RA8876_REG_FGCB,   color.b);

  // draw
  regWrite(reg, cmd);

  // wait for completion
  waitUntilStatusIdle();

  SPI.endTransaction();
}

void
RA8876::drawEllipseShape(uint16_t x, uint16_t y, uint16_t xrad, uint16_t yrad, Color color, uint8_t reg, uint8_t cmd)
{
  SPI.beginTransaction(m_spiSettings);

  // first point
  regWrite(RA8876_REG_DEHR0,   x       & 0xff);
  regWrite(RA8876_REG_DEHR1,  (x >> 8) & 0xff);
  regWrite(RA8876_REG_DEVR0,   y       & 0xff);
  regWrite(RA8876_REG_DEVR1,  (y >> 8) & 0xff);

  // radii
  regWrite(RA8876_REG_ELL_A0,  xrad       & 0xff);
  regWrite(RA8876_REG_ELL_A1, (xrad >> 8) & 0xff);
  regWrite(RA8876_REG_ELL_B0,  yrad       & 0xff);
  regWrite(RA8876_REG_ELL_B1, (yrad >> 8) & 0xff);

  // colour
  regWrite(RA8876_REG_FGCR,    color.r);
  regWrite(RA8876_REG_FGCG,    color.g);
  regWrite(RA8876_REG_FGCB,    color.b);

  // draw
  regWrite(reg, cmd);

  // wait for completion
  waitUntilStatusIdle();

  SPI.endTransaction();
}

//-------------------------------------------------------------------------