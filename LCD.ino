//lcd 128 x 160 px
void showInfoLcd()
{
  //commSerial.printf("show EPAPER");
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);

  display.setCursor(0, 19);

  //---main voltage---
  display.setFont(&FreeSansBold14pt7b);
  display.print((float)packBasicInfo.Volts / 1000);

  display.println("V");
  display.setFont(&FreeMonoBold9pt7b);
  /*
    //---single cell voltage---
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.print("(");
    tft.print((float)packCellInfo.CellMedian / 1000);
    tft.print("V");
    tft.print(")");
    tft.println();
  */
  //---main current---
  display.setFont(&FreeMonoBold12pt7b);
  display.print((float)packBasicInfo.Amps / 1000);
  display.println("A ");

  display.setFont(&FreeMonoBold9pt7b);
  int Moss = packBasicInfo.MosfetStatus;
  if (Moss == 1) { //OFF
    display.print("MOSS:");
    display.print("OFF");
    display.println();
    display.println();
  }
  else if (Moss == 3) {// ON
    display.print("MOSS:");
    display.print("ON");
    display.println();
    display.println();
  }
  /*
    //---ampere hours---
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.print((float)packBasicInfo.CapacityRemainAh / 1000);
    tft.print("Ah");
    tft.println();
  */

  //---watts---
  display.setFont(&FreeMonoBold12pt7b);
  display.print(packBasicInfo.Watts);
  display.print("W    ");
  display.println();

  //---battery percent---
  display.setFont(&FreeMonoBold12pt7b);
  display.print(packBasicInfo.CapacityRemainPercent);
  display.println("%");


  //---battery Wh---
  display.print(packBasicInfo.CapacityRemainWh);
  display.print("Wh");
  display.println();
  //display.println();

  //---temperatures---
  display.setFont(&FreeMonoBold9pt7b);
  display.print("T1: ");
  display.print((float)packBasicInfo.Temp1 / 10);
  display.println("C");
  display.print("T2: ");
  display.print((float)packBasicInfo.Temp2 / 10);
  display.print("C");
  display.println();

  display.setCursor(80, 245);
  //display.print("bat:");
  float Vbat = (((3.3 / 4095) * analogRead(35)) * 2) + 0.22;
  int VbatP = mapf(Vbat, 3.0, 4.2, 0, 100); // vbat en pourcentage
  display.print(VbatP);
  display.println("%");
  //display.print("batd:");
  //display.println(analogRead(35));
  // display.update();
  display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, true);

}

void lcdConnectingStatus(uint8_t state)
{
  display.setFont(&FreeSansBold8pt7b);
  switch (state)
  {
    case 0:
      display.println("connecting to BMS...");
      break;
    case 1:
      display.println("created client");
      break;
    case 2:
      display.println("connected to server");
      break;
    case 3:
      display.println("service not found");
      break;
    case 4:
      display.println("found service");
      break;
    case 5:
      display.println("char. not found");
      break;
    case 6:
      display.println("char. found");
      break;
    default:
      break;

  }
  display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, true);
}
double mapf(double val, double in_min, double in_max, double out_min, double out_max) {
  return (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
