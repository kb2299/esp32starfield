#define LGFX_ESP_WROVER_KIT
#define LGFX_USE_V1

#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_GC9A01 _panel_instance;
  lgfx::Bus_SPI _bus_instance;

public:
  LGFX(void)
  {
    {
      auto cfg = _bus_instance.config();

      cfg.spi_host = HSPI_HOST;
      cfg.spi_3wire = true;
      cfg.use_lock = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk = 14;
      cfg.pin_mosi = 15;
      cfg.pin_miso = -1;
      cfg.pin_dc = 27;

      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto cfg = _panel_instance.config();

      cfg.pin_cs = 5;
      cfg.pin_rst = 33;
      cfg.pin_busy = -1;

      cfg.panel_width = 240;
      cfg.panel_height = 240;
      cfg.invert           = false;
      // cfg.rgb_order        = false;
      // cfg.dlen_16bit       = false;
      cfg.bus_shared = false;

      _panel_instance.config(cfg);
    }
    setPanel(&_panel_instance);
  }
};

static LGFX lcd;

static std::int32_t lcd_width;
static std::int32_t lcd_height;

static LGFX_Sprite buffer[2];

#define NUMBER_OF_STARS 2000

static std::uint16_t centerx, centery;

typedef struct
{
  float xpos, ypos, zpos;
  short speed;
} STAR;

static STAR stars[NUMBER_OF_STARS];

void init_star(STAR *star, int zpos)
{
  star->xpos = -10.0 + (20.0 * (rand() / (RAND_MAX + 1.0)));
  star->ypos = -10.0 + (20.0 * (rand() / (RAND_MAX + 1.0)));

  star->xpos *= 3000.0;
  star->ypos *= 3000.0;

  star->zpos = zpos;
  star->speed = 2 + (int)(2.0 * (rand() / (RAND_MAX + 1.0)));
}


void setup(void)
{
  lcd.init();
  lcd.setColorDepth(8);
  lcd.setRotation(2);

  lcd_width = lcd.width();
  lcd_height = lcd.height();

  centerx = lcd_width / 2;
  centery = lcd_width / 2;

  for (int i = 0; i < NUMBER_OF_STARS; ++i)
  {
    init_star(stars + i, 1+ rand() % 3000);
  }

  for (std::uint32_t i = 0; i < 2; ++i)
  {
    buffer[i].setColorDepth(8);
    buffer[i].createSprite(lcd_width, lcd_height);
    // buffer[i].setPaletteGrayscale();
  }

  lcd.setFont(&fonts::Font2);

  // Занимаем SPI под отрисовку
  lcd.startWrite();
}

static std::uint16_t col;
void loop(void)
{
  static int i, tempx, tempy;

  static short flip = 0;

  //выбираем буфер для отрисовки
  flip = flip ? 0 : 1;

  buffer[flip].clear(lcd.color565(255, 255, 255));

  for (i = 0; i < NUMBER_OF_STARS; i++)
  {
    stars[i].zpos -= stars[i].speed;

    int u = 255 - (255 / stars[i].zpos) * 100;
    col = lcd.color565(u, u, u);

    if (stars[i].zpos <= 0)
    {
      init_star(stars + i, 3000);
    }

    tempx = (stars[i].xpos / stars[i].zpos) + centerx;
    tempy = (stars[i].ypos / stars[i].zpos) + centery;

    if (tempx < 0 || tempx > lcd_width - 1 || tempy < 0 || tempy > lcd_height - 1)
    {
      init_star(stars + i, 3000);
      continue;
    }

    buffer[flip].writePixel(tempx, tempy, col);
    buffer[flip].writePixel(tempx + 1, tempy, col);
    buffer[flip].writePixel(tempx + 1, tempy + 1, col);
    buffer[flip].writePixel(tempx, tempy + 1, col);
  }

  // посылаем буфер на отрисовку
  // (двойная буферизация собственно)
  buffer[flip].pushSprite(&lcd, 0, 0);

  // lcd.display();
}