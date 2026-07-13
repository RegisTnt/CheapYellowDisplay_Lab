#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <TouchController.h>

namespace
{

constexpr uint8_t TOUCH_MOSI_PIN = 32;
constexpr uint8_t TOUCH_MISO_PIN = 39;
constexpr uint8_t TOUCH_SCLK_PIN = 25;
constexpr uint8_t TOUCH_CS_PIN = 33;
constexpr uint8_t TOUCH_IRQ_PIN = 36;

// Valeurs provisoires : elles doivent être remplacées après mesure sur la carte.
constexpr int16_t TOUCH_RAW_MIN_X = 200;
constexpr int16_t TOUCH_RAW_MAX_X = 3800;
constexpr int16_t TOUCH_RAW_MIN_Y = 200;
constexpr int16_t TOUCH_RAW_MAX_Y = 3800;

constexpr uint32_t TOUCH_READ_INTERVAL_MS = 30;
constexpr int16_t CROSSHAIR_RADIUS = 6;

TFT_eSPI tft;
SPIClass touchSpi(HSPI);
CYDTouch::TouchController touchController(touchSpi, TOUCH_CS_PIN, TOUCH_IRQ_PIN);
bool touchReady = false;

void drawHeader()
{
    tft.fillRect(0, 0, tft.width(), 45, TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("CYD Touch Lab", tft.width() / 2, 8, 4);
    tft.drawFastHLine(10, 42, tft.width() - 20, TFT_DARKGREY);
}

void drawInformation(const CYDTouch::TouchPoint& point)
{
    char line[48];

    tft.fillRect(8, 50, 304, 118, TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    snprintf(line, sizeof(line), "Etat : %s", point.touched ? "TOUCH" : "RELEASED");
    tft.drawString(line, 12, 52, 2);
    snprintf(line, sizeof(line), "Ecran : X=%d  Y=%d", point.x, point.y);
    tft.drawString(line, 12, 80, 2);
    snprintf(line, sizeof(line), "Brut : X=%d  Y=%d", point.rawX, point.rawY);
    tft.drawString(line, 12, 108, 2);
    snprintf(line, sizeof(line), "Pression : %d", point.pressure);
    tft.drawString(line, 12, 136, 2);
}

void drawCrosshair(int16_t x, int16_t y, uint16_t color)
{
    tft.drawCircle(x, y, CROSSHAIR_RADIUS, color);
    tft.drawFastHLine(x - CROSSHAIR_RADIUS - 3, y, 2 * CROSSHAIR_RADIUS + 7, color);
    tft.drawFastVLine(x, y - CROSSHAIR_RADIUS - 3, 2 * CROSSHAIR_RADIUS + 7, color);
}

bool pointsDiffer(
    const CYDTouch::TouchPoint& current,
    const CYDTouch::TouchPoint& previous
)
{
    return current.touched != previous.touched
        || current.x != previous.x
        || current.y != previous.y
        || current.rawX != previous.rawX
        || current.rawY != previous.rawY
        || current.pressure != previous.pressure;
}

void printTouchPoint(const CYDTouch::TouchPoint& point)
{
    Serial.printf(
        "rawX=%d rawY=%d pressure=%d screenX=%d screenY=%d\n",
        point.rawX,
        point.rawY,
        point.pressure,
        point.x,
        point.y
    );
}

}  // namespace

void setup()
{
    Serial.begin(115200);
    delay(500);

    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    drawHeader();

    // Le tactile utilise HSPI avec ses propres broches, distinctes du bus TFT.
    touchSpi.begin(TOUCH_SCLK_PIN, TOUCH_MISO_PIN, TOUCH_MOSI_PIN, TOUCH_CS_PIN);
    touchController.setScreenSize(tft.width(), tft.height());
    touchController.setCalibration(
        TOUCH_RAW_MIN_X,
        TOUCH_RAW_MAX_X,
        TOUCH_RAW_MIN_Y,
        TOUCH_RAW_MAX_Y
    );

    touchReady = touchController.begin();
    if (!touchReady)
    {
        Serial.println("XPT2046 initialization failed.");
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawString("Touch init failed", 12, 52, 2);
        return;
    }

    const CYDTouch::TouchPoint releasedPoint = {0, 0, 0, 0, 0, false};
    drawInformation(releasedPoint);
    Serial.println("CYD touch demo ready.");
}

void loop()
{
    static uint32_t lastReadTime = 0;
    static CYDTouch::TouchPoint previousPoint = {0, 0, 0, 0, 0, false};

    if (!touchReady)
    {
        return;
    }

    const uint32_t now = millis();
    if (now - lastReadTime < TOUCH_READ_INTERVAL_MS)
    {
        return;
    }
    lastReadTime = now;

    CYDTouch::TouchPoint currentPoint;
    touchController.read(currentPoint);

    if (!pointsDiffer(currentPoint, previousPoint))
    {
        return;
    }

    if (previousPoint.touched)
    {
        drawCrosshair(previousPoint.x, previousPoint.y, TFT_BLACK);
        drawHeader();
    }

    drawInformation(currentPoint);

    if (currentPoint.touched)
    {
        drawCrosshair(currentPoint.x, currentPoint.y, TFT_YELLOW);
        printTouchPoint(currentPoint);
    }

    previousPoint = currentPoint;
}
