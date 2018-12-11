EESchema Schematic File Version 4
LIBS:MainBoard-cache
EELAYER 26 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 3
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L MainBoard:STM32F334K8 U5
U 1 1 5AEB44C0
P 5200 3500
F 0 "U5" H 5200 4365 50  0000 C CNN
F 1 "STM32F334K8" H 5200 4274 50  0000 C CNN
F 2 "Package_QFP:LQFP-32_7x7mm_P0.8mm" H 5200 2950 50  0001 C CNN
F 3 "www.st.com/content/ccc/resource/technical/document/datasheet/d1/cd/3d/18/a2/2c/4e/d0/DM00097745.pdf/files/DM00097745.pdf/jcr:content/translations/en.DM00097745.pdf" H 5200 2950 50  0001 C CNN
	1    5200 3500
	1    0    0    -1  
$EndComp
$Comp
L MainBoard:STM32F334K8 U5
U 2 1 5AEB4808
P 5200 4850
F 0 "U5" H 5200 5365 50  0000 C CNN
F 1 "STM32F334K8" H 5200 5274 50  0000 C CNN
F 2 "Package_QFP:LQFP-32_7x7mm_P0.8mm" H 5200 4300 50  0001 C CNN
F 3 "www.st.com/content/ccc/resource/technical/document/datasheet/d1/cd/3d/18/a2/2c/4e/d0/DM00097745.pdf/files/DM00097745.pdf/jcr:content/translations/en.DM00097745.pdf" H 5200 4300 50  0001 C CNN
	2    5200 4850
	1    0    0    -1  
$EndComp
$Comp
L MainBoard:M24M02 U3
U 1 1 5AEB4FB4
P 1700 2300
F 0 "U3" H 1700 2715 50  0000 C CNN
F 1 "M24M02" H 1700 2624 50  0000 C CNN
F 2 "MainBoard:SON-8" H 1700 2000 50  0001 C CNN
F 3 "http://www.st.com/resource/en/datasheet/m24m02-dr.pdf" H 1700 2300 50  0001 C CNN
	1    1700 2300
	1    0    0    -1  
$EndComp
$Comp
L MainBoard:M24M02 U4
U 1 1 5AEB4FEA
P 1700 3100
F 0 "U4" H 1700 3515 50  0000 C CNN
F 1 "M24M02" H 1700 3424 50  0000 C CNN
F 2 "MainBoard:SON-8" H 1700 2800 50  0001 C CNN
F 3 "http://www.st.com/resource/en/datasheet/m24m02-dr.pdf" H 1700 3100 50  0001 C CNN
	1    1700 3100
	1    0    0    -1  
$EndComp
$Comp
L MainBoard:MCP2562 U7
U 1 1 5AEC374E
P 8600 4050
F 0 "U7" H 8600 4515 50  0000 C CNN
F 1 "MCP2562" H 8600 4424 50  0000 C CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 8600 3700 50  0001 C CNN
F 3 "http://ww1.microchip.com/downloads/en/DeviceDoc/20005167C.pdf" H 8600 4050 50  0001 C CNN
	1    8600 4050
	1    0    0    -1  
$EndComp
$Comp
L MainBoard:MicroSD U6
U 1 1 5AEC3D01
P 1700 3900
F 0 "U6" H 1700 4315 50  0000 C CNN
F 1 "MicroSD" H 1700 4224 50  0000 C CNN
F 2 "MainBoard:Molex_MicroSD_PushPull" H 1700 3900 50  0001 C CNN
F 3 "https://www.molex.com/molex/products/datasheet.jsp?part=active/1040310811_MEMORY_CARD_SOCKET.xml&channel=Products&Lang=en-US" H 1700 3900 50  0001 C CNN
	1    1700 3900
	1    0    0    -1  
$EndComp
Text Label 4300 3750 0    50   ~ 0
SCL
Text Label 4300 3650 0    50   ~ 0
MOSI
Text Label 4300 3550 0    50   ~ 0
MISO
Text Label 4300 3450 0    50   ~ 0
SCK
Wire Wire Line
	4300 3450 4600 3450
Wire Wire Line
	4300 3650 4600 3650
Wire Wire Line
	2300 2150 2400 2150
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR018
U 1 1 5AEC9B62
P 2400 2150
F 0 "#PWR018" H 2400 1900 50  0001 C CNN
F 1 "GND" V 2400 2000 50  0000 R CNN
F 2 "" H 2400 2150 50  0001 C CNN
F 3 "" H 2400 2150 50  0001 C CNN
	1    2400 2150
	0    -1   -1   0   
$EndComp
Wire Wire Line
	2300 2950 2400 2950
$Comp
L MainBoard-rescue:+3.3V-power-MainBoard-rescue #PWR022
U 1 1 5AEC9D0C
P 2400 2950
F 0 "#PWR022" H 2400 2800 50  0001 C CNN
F 1 "+3.3V" V 2400 3100 50  0000 L CNN
F 2 "" H 2400 2950 50  0001 C CNN
F 3 "" H 2400 2950 50  0001 C CNN
	1    2400 2950
	0    1    1    0   
$EndComp
Wire Wire Line
	1100 3000 1000 3000
Wire Wire Line
	1100 2200 1000 2200
$Comp
L MainBoard-rescue:+3.3V-power-MainBoard-rescue #PWR019
U 1 1 5AECC49E
P 1000 2200
F 0 "#PWR019" H 1000 2050 50  0001 C CNN
F 1 "+3.3V" V 1000 2350 50  0000 L CNN
F 2 "" H 1000 2200 50  0001 C CNN
F 3 "" H 1000 2200 50  0001 C CNN
	1    1000 2200
	0    -1   -1   0   
$EndComp
$Comp
L MainBoard-rescue:+3.3V-power-MainBoard-rescue #PWR023
U 1 1 5AECC4F2
P 1000 3000
F 0 "#PWR023" H 1000 2850 50  0001 C CNN
F 1 "+3.3V" V 1000 3150 50  0000 L CNN
F 2 "" H 1000 3000 50  0001 C CNN
F 3 "" H 1000 3000 50  0001 C CNN
	1    1000 3000
	0    -1   -1   0   
$EndComp
Wire Wire Line
	1100 2400 1000 2400
Wire Wire Line
	1100 3200 1000 3200
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR020
U 1 1 5AECCE9C
P 1000 2400
F 0 "#PWR020" H 1000 2150 50  0001 C CNN
F 1 "GND" V 1000 2250 50  0000 R CNN
F 2 "" H 1000 2400 50  0001 C CNN
F 3 "" H 1000 2400 50  0001 C CNN
	1    1000 2400
	0    1    1    0   
$EndComp
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR025
U 1 1 5AECCEF0
P 1000 3200
F 0 "#PWR025" H 1000 2950 50  0001 C CNN
F 1 "GND" V 1000 3050 50  0000 R CNN
F 2 "" H 1000 3200 50  0001 C CNN
F 3 "" H 1000 3200 50  0001 C CNN
	1    1000 3200
	0    1    1    0   
$EndComp
Text Label 2600 3750 2    50   ~ 0
MOSI
Text Label 2600 3850 2    50   ~ 0
MISO
Text Label 2600 3950 2    50   ~ 0
SCK
Wire Wire Line
	1100 3750 1000 3750
$Comp
L MainBoard-rescue:+3.3V-power-MainBoard-rescue #PWR028
U 1 1 5AED0C3F
P 1000 3750
F 0 "#PWR028" H 1000 3600 50  0001 C CNN
F 1 "+3.3V" V 1000 3900 50  0000 L CNN
F 2 "" H 1000 3750 50  0001 C CNN
F 3 "" H 1000 3750 50  0001 C CNN
	1    1000 3750
	0    -1   -1   0   
$EndComp
Wire Wire Line
	1100 3850 1000 3850
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR029
U 1 1 5AED0C46
P 1000 3850
F 0 "#PWR029" H 1000 3600 50  0001 C CNN
F 1 "GND" V 1000 3700 50  0000 R CNN
F 2 "" H 1000 3850 50  0001 C CNN
F 3 "" H 1000 3850 50  0001 C CNN
	1    1000 3850
	0    1    1    0   
$EndComp
$Comp
L MainBoard:LTC6820 U1
U 1 1 5AED2E92
P 7200 1400
F 0 "U1" H 7200 2115 50  0000 C CNN
F 1 "LTC6820" H 7200 2024 50  0000 C CNN
F 2 "MainBoard:MSOP-16" H 7200 800 50  0001 C CNN
F 3 "http://cds.linear.com/docs/en/datasheet/6820fb.pdf" H 7200 1400 50  0001 C CNN
	1    7200 1400
	1    0    0    -1  
$EndComp
$Comp
L MainBoard:HM2103 U2
U 1 1 5AED5C4E
P 8650 1700
F 0 "U2" H 8650 2025 50  0000 C CNN
F 1 "HM2103" H 8650 1934 50  0000 C CNN
F 2 "MainBoard:HM2103" H 8650 1500 50  0001 C CNN
F 3 "https://productfinder.pulseeng.com/doc_type/WEB301/doc_num/HM2103NL/doc_part/HM2103NL.pdf" H 8750 1650 50  0001 C CNN
	1    8650 1700
	-1   0    0    1   
$EndComp
Wire Notes Line
	600  1800 600  4200
Wire Notes Line
	600  4200 2800 4200
Wire Notes Line
	2800 4200 2800 1800
Wire Notes Line
	2800 1800 600  1800
Text Notes 600  1800 0    50   ~ 0
MEMORY
Wire Wire Line
	6600 950  6500 950 
Wire Wire Line
	6600 1150 6500 1150
Wire Wire Line
	6600 1350 6500 1350
Wire Wire Line
	6600 1450 6500 1450
Wire Wire Line
	6600 1750 6500 1750
Wire Wire Line
	6600 1850 6500 1850
$Comp
L MainBoard-rescue:+5V-power-MainBoard-rescue #PWR02
U 1 1 5AEDE661
P 6500 950
F 0 "#PWR02" H 6500 800 50  0001 C CNN
F 1 "+5V" V 6500 1100 50  0000 L CNN
F 2 "" H 6500 950 50  0001 C CNN
F 3 "" H 6500 950 50  0001 C CNN
	1    6500 950 
	0    -1   -1   0   
$EndComp
$Comp
L MainBoard-rescue:+3.3V-power-MainBoard-rescue #PWR04
U 1 1 5AEDE76F
P 6500 1050
F 0 "#PWR04" H 6500 900 50  0001 C CNN
F 1 "+3.3V" V 6500 1200 50  0000 L CNN
F 2 "" H 6500 1050 50  0001 C CNN
F 3 "" H 6500 1050 50  0001 C CNN
	1    6500 1050
	0    -1   -1   0   
$EndComp
Wire Wire Line
	6500 1050 6600 1050
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR06
U 1 1 5AEDE83D
P 6500 1150
F 0 "#PWR06" H 6500 900 50  0001 C CNN
F 1 "GND" V 6500 1000 50  0000 R CNN
F 2 "" H 6500 1150 50  0001 C CNN
F 3 "" H 6500 1150 50  0001 C CNN
	1    6500 1150
	0    1    1    0   
$EndComp
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR09
U 1 1 5AEDE9E5
P 6500 1350
F 0 "#PWR09" H 6500 1100 50  0001 C CNN
F 1 "GND" V 6500 1200 50  0000 R CNN
F 2 "" H 6500 1350 50  0001 C CNN
F 3 "" H 6500 1350 50  0001 C CNN
	1    6500 1350
	0    1    1    0   
$EndComp
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR011
U 1 1 5AEDEA06
P 6500 1450
F 0 "#PWR011" H 6500 1200 50  0001 C CNN
F 1 "GND" V 6500 1300 50  0000 R CNN
F 2 "" H 6500 1450 50  0001 C CNN
F 3 "" H 6500 1450 50  0001 C CNN
	1    6500 1450
	0    1    1    0   
$EndComp
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR016
U 1 1 5AEDEA27
P 6500 1750
F 0 "#PWR016" H 6500 1500 50  0001 C CNN
F 1 "GND" V 6500 1600 50  0000 R CNN
F 2 "" H 6500 1750 50  0001 C CNN
F 3 "" H 6500 1750 50  0001 C CNN
	1    6500 1750
	0    1    1    0   
$EndComp
$Comp
L MainBoard-rescue:+5V-power-MainBoard-rescue #PWR017
U 1 1 5AEDEA48
P 6500 1850
F 0 "#PWR017" H 6500 1700 50  0001 C CNN
F 1 "+5V" V 6500 2000 50  0000 L CNN
F 2 "" H 6500 1850 50  0001 C CNN
F 3 "" H 6500 1850 50  0001 C CNN
	1    6500 1850
	0    -1   -1   0   
$EndComp
Wire Wire Line
	7800 1150 8000 1150
Wire Wire Line
	7800 1250 8000 1250
Wire Wire Line
	7800 1350 8000 1350
Wire Wire Line
	7800 1450 7900 1450
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR012
U 1 1 5AEE27DE
P 7900 1450
F 0 "#PWR012" H 7900 1200 50  0001 C CNN
F 1 "GND" V 7900 1300 50  0000 R CNN
F 2 "" H 7900 1450 50  0001 C CNN
F 3 "" H 7900 1450 50  0001 C CNN
	1    7900 1450
	0    -1   -1   0   
$EndComp
Text Label 8000 1250 2    50   ~ 0
SCK
Text Label 8000 1050 2    50   ~ 0
MISO
Text Label 8000 1150 2    50   ~ 0
MOSI
Text Label 8000 1650 2    50   ~ 0
I+
Text Label 8000 1750 2    50   ~ 0
I-
Wire Wire Line
	7800 1650 8300 1650
Wire Wire Line
	7800 1750 8300 1750
NoConn ~ 8950 1700
Wire Wire Line
	8950 1600 9050 1600
Wire Wire Line
	8950 1800 9050 1800
$Comp
L MainBoard-rescue:R-Device-MainBoard-rescue R4
U 1 1 5AEF681F
P 9150 1700
F 0 "R4" V 9250 1700 50  0000 C CNN
F 1 "200" V 9150 1700 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 9080 1700 50  0001 C CNN
F 3 "~" H 9150 1700 50  0001 C CNN
	1    9150 1700
	-1   0    0    1   
$EndComp
Text Label 9550 1500 2    50   ~ 0
isoSPI+
Text Label 9550 1900 2    50   ~ 0
isoSPI-
$Comp
L MainBoard-rescue:Conn_01x02-Connector_Generic-MainBoard-rescue J1
U 1 1 5AF07F56
P 9850 1750
F 0 "J1" H 9850 1850 50  0000 C CNN
F 1 "isoSPI" V 9950 1700 50  0000 C CNN
F 2 "Connector_Molex:Molex_Micro-Fit_3.0_43650-0215_1x02_P3.00mm_Vertical" H 9850 1750 50  0001 C CNN
F 3 "~" H 9850 1750 50  0001 C CNN
	1    9850 1750
	1    0    0    1   
$EndComp
Text Label 6400 1550 0    50   ~ 0
IBIAS
Wire Wire Line
	6400 1550 6600 1550
Wire Wire Line
	6600 1650 6400 1650
Text Label 6400 1650 0    50   ~ 0
ICMP
Wire Wire Line
	8500 1150 8700 1150
$Comp
L MainBoard-rescue:R-Device-MainBoard-rescue R1
U 1 1 5AF1410D
P 8850 1150
F 0 "R1" V 8750 1150 50  0000 C CNN
F 1 "604" V 8850 1150 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 8780 1150 50  0001 C CNN
F 3 "~" H 8850 1150 50  0001 C CNN
	1    8850 1150
	0    1    1    0   
$EndComp
Wire Wire Line
	9000 1150 9300 1150
$Comp
L MainBoard-rescue:R-Device-MainBoard-rescue R2
U 1 1 5AF15F09
P 9450 1150
F 0 "R2" V 9350 1150 50  0000 C CNN
F 1 "1.4K" V 9450 1150 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 9380 1150 50  0001 C CNN
F 3 "~" H 9450 1150 50  0001 C CNN
	1    9450 1150
	0    1    1    0   
$EndComp
Wire Wire Line
	9600 1150 9700 1150
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR07
U 1 1 5AF17DE6
P 9700 1150
F 0 "#PWR07" H 9700 900 50  0001 C CNN
F 1 "GND" V 9700 1000 50  0000 R CNN
F 2 "" H 9700 1150 50  0001 C CNN
F 3 "" H 9700 1150 50  0001 C CNN
	1    9700 1150
	0    -1   -1   0   
$EndComp
Text Label 8500 1150 0    50   ~ 0
IBIAS
Text Label 9050 1150 0    50   ~ 0
ICMP
Wire Notes Line
	6100 600  11200 600 
Wire Notes Line
	11200 600  11200 2100
Wire Notes Line
	11200 2100 6100 2100
Wire Notes Line
	6100 2100 6100 600 
Text Notes 6100 600  0    50   ~ 0
isoSPI INTERFACE
$Comp
L MainBoard-rescue:R-Device-MainBoard-rescue R8
U 1 1 5AF1C2A6
P 10050 4400
F 0 "R8" V 10150 4400 50  0000 C CNN
F 1 "60" V 10050 4400 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 9980 4400 50  0001 C CNN
F 3 "~" H 10050 4400 50  0001 C CNN
	1    10050 4400
	-1   0    0    1   
$EndComp
Text Label 9850 4150 2    50   ~ 0
CAN+
Text Label 9850 4250 2    50   ~ 0
CAN-
Wire Wire Line
	9200 3950 9300 3950
Wire Wire Line
	9200 3850 9300 3850
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR032
U 1 1 5AF34057
P 9300 3950
F 0 "#PWR032" H 9300 3700 50  0001 C CNN
F 1 "GND" V 9300 3800 50  0000 R CNN
F 2 "" H 9300 3950 50  0001 C CNN
F 3 "" H 9300 3950 50  0001 C CNN
	1    9300 3950
	0    -1   -1   0   
$EndComp
$Comp
L MainBoard-rescue:+5V-power-MainBoard-rescue #PWR030
U 1 1 5AF340B8
P 9300 3850
F 0 "#PWR030" H 9300 3700 50  0001 C CNN
F 1 "+5V" V 9300 4000 50  0000 L CNN
F 2 "" H 9300 3850 50  0001 C CNN
F 3 "" H 9300 3850 50  0001 C CNN
	1    9300 3850
	0    1    1    0   
$EndComp
$Comp
L MainBoard-rescue:+3.3V-power-MainBoard-rescue #PWR031
U 1 1 5AF34113
P 7900 3900
F 0 "#PWR031" H 7900 3750 50  0001 C CNN
F 1 "+3.3V" V 7900 4050 50  0000 L CNN
F 2 "" H 7900 3900 50  0001 C CNN
F 3 "" H 7900 3900 50  0001 C CNN
	1    7900 3900
	0    -1   -1   0   
$EndComp
Wire Wire Line
	7900 3900 8000 3900
Wire Wire Line
	5800 4000 8000 4000
Wire Wire Line
	5800 4100 8000 4100
Wire Wire Line
	8000 4200 7900 4200
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR034
U 1 1 5AF4D610
P 7900 4200
F 0 "#PWR034" H 7900 3950 50  0001 C CNN
F 1 "GND" V 7900 4050 50  0000 R CNN
F 2 "" H 7900 4200 50  0001 C CNN
F 3 "" H 7900 4200 50  0001 C CNN
	1    7900 4200
	0    1    1    0   
$EndComp
Wire Notes Line
	7500 3500 7500 4500
Wire Notes Line
	7500 4500 10400 4500
Wire Notes Line
	10400 4500 10400 3500
Wire Notes Line
	10400 3500 7500 3500
Text Notes 7500 3500 0    50   ~ 0
CAN INTERFACE
Wire Wire Line
	5800 4600 5900 4600
Wire Wire Line
	5800 4700 5900 4700
Wire Wire Line
	5800 4800 5900 4800
Wire Wire Line
	5800 5000 5900 5000
Wire Wire Line
	5800 5100 5900 5100
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR043
U 1 1 5AF6012A
P 5900 5100
F 0 "#PWR043" H 5900 4850 50  0001 C CNN
F 1 "GND" V 5900 4950 50  0000 R CNN
F 2 "" H 5900 5100 50  0001 C CNN
F 3 "" H 5900 5100 50  0001 C CNN
	1    5900 5100
	0    -1   -1   0   
$EndComp
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR041
U 1 1 5AF601DF
P 5900 5000
F 0 "#PWR041" H 5900 4750 50  0001 C CNN
F 1 "GND" V 5900 4850 50  0000 R CNN
F 2 "" H 5900 5000 50  0001 C CNN
F 3 "" H 5900 5000 50  0001 C CNN
	1    5900 5000
	0    -1   -1   0   
$EndComp
$Comp
L MainBoard-rescue:+3.3V-power-MainBoard-rescue #PWR035
U 1 1 5AF60212
P 5900 4600
F 0 "#PWR035" H 5900 4450 50  0001 C CNN
F 1 "+3.3V" V 5900 4750 50  0000 L CNN
F 2 "" H 5900 4600 50  0001 C CNN
F 3 "" H 5900 4600 50  0001 C CNN
	1    5900 4600
	0    1    1    0   
$EndComp
$Comp
L MainBoard-rescue:+3.3V-power-MainBoard-rescue #PWR036
U 1 1 5AF6026D
P 5900 4700
F 0 "#PWR036" H 5900 4550 50  0001 C CNN
F 1 "+3.3V" V 5900 4850 50  0000 L CNN
F 2 "" H 5900 4700 50  0001 C CNN
F 3 "" H 5900 4700 50  0001 C CNN
	1    5900 4700
	0    1    1    0   
$EndComp
Wire Wire Line
	4600 5000 4500 5000
$Comp
L MainBoard-rescue:+3.3V-power-MainBoard-rescue #PWR01
U 1 1 5AF67586
P 900 550
F 0 "#PWR01" H 900 400 50  0001 C CNN
F 1 "+3.3V" V 900 700 50  0000 L CNN
F 2 "" H 900 550 50  0001 C CNN
F 3 "" H 900 550 50  0001 C CNN
	1    900  550 
	0    -1   -1   0   
$EndComp
Wire Wire Line
	900  550  1100 550 
Wire Wire Line
	1100 550  1100 650 
$Comp
L MainBoard-rescue:C-Device-MainBoard-rescue C14
U 1 1 5AF6B0F4
P 1500 1400
F 0 "C14" V 1350 1400 50  0000 C CNN
F 1 "10n" V 1650 1400 50  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 1538 1250 50  0001 C CNN
F 3 "~" H 1500 1400 50  0001 C CNN
	1    1500 1400
	1    0    0    -1  
$EndComp
Wire Wire Line
	1100 950  1100 1050
Wire Wire Line
	1100 1050 900  1050
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR03
U 1 1 5AF6EE27
P 900 1050
F 0 "#PWR03" H 900 800 50  0001 C CNN
F 1 "GND" V 900 900 50  0000 R CNN
F 2 "" H 900 1050 50  0001 C CNN
F 3 "" H 900 1050 50  0001 C CNN
	1    900  1050
	0    1    1    0   
$EndComp
Wire Wire Line
	1100 550  1500 550 
Connection ~ 1100 550 
Wire Wire Line
	1100 1050 1500 1050
Connection ~ 1100 1050
$Comp
L MainBoard-rescue:C-Device-MainBoard-rescue C6
U 1 1 5AF7B313
P 3100 800
F 0 "C6" V 2950 800 50  0000 C CNN
F 1 "10n" V 3250 800 50  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 3138 650 50  0001 C CNN
F 3 "~" H 3100 800 50  0001 C CNN
	1    3100 800 
	1    0    0    -1  
$EndComp
Wire Wire Line
	1500 550  1500 650 
Wire Wire Line
	1500 950  1500 1050
Wire Wire Line
	1500 550  1900 550 
Wire Wire Line
	1900 550  1900 650 
Connection ~ 1500 550 
Wire Wire Line
	1900 950  1900 1050
Wire Wire Line
	1900 1050 1500 1050
Connection ~ 1500 1050
$Comp
L MainBoard-rescue:C-Device-MainBoard-rescue C8
U 1 1 5AF8C43D
P 3900 800
F 0 "C8" V 3750 800 50  0000 C CNN
F 1 "10n" V 4050 800 50  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 3938 650 50  0001 C CNN
F 3 "~" H 3900 800 50  0001 C CNN
	1    3900 800 
	1    0    0    -1  
$EndComp
Wire Wire Line
	1900 550  2300 550 
Wire Wire Line
	2300 550  2300 650 
Connection ~ 1900 550 
Wire Wire Line
	2300 1050 2300 950 
Wire Wire Line
	1900 1050 2300 1050
Connection ~ 1900 1050
$Comp
L MainBoard-rescue:C-Device-MainBoard-rescue C3
U 1 1 5AF9A948
P 1900 800
F 0 "C3" V 1750 800 50  0000 C CNN
F 1 "100n" V 2050 800 50  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 1938 650 50  0001 C CNN
F 3 "~" H 1900 800 50  0001 C CNN
	1    1900 800 
	1    0    0    -1  
$EndComp
Wire Wire Line
	900  1150 1100 1150
Wire Wire Line
	1100 1150 1100 1250
Wire Wire Line
	1100 1650 900  1650
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR014
U 1 1 5AF9ACAE
P 900 1650
F 0 "#PWR014" H 900 1400 50  0001 C CNN
F 1 "GND" V 900 1500 50  0000 R CNN
F 2 "" H 900 1650 50  0001 C CNN
F 3 "" H 900 1650 50  0001 C CNN
	1    900  1650
	0    1    1    0   
$EndComp
Connection ~ 1100 1150
Connection ~ 1100 1650
Wire Wire Line
	1500 1150 1500 1250
Wire Wire Line
	2700 1550 2700 1650
Wire Wire Line
	1900 1150 2300 1150
Wire Wire Line
	1900 1150 1900 1250
Connection ~ 1900 1150
Wire Wire Line
	3100 1650 3100 1550
$Comp
L MainBoard-rescue:+5V-power-MainBoard-rescue #PWR05
U 1 1 5AFA0EE2
P 900 1150
F 0 "#PWR05" H 900 1000 50  0001 C CNN
F 1 "+5V" V 900 1300 50  0000 L CNN
F 2 "" H 900 1150 50  0001 C CNN
F 3 "" H 900 1150 50  0001 C CNN
	1    900  1150
	0    -1   -1   0   
$EndComp
Wire Wire Line
	1100 1550 1100 1650
$Comp
L MainBoard-rescue:C-Device-MainBoard-rescue C15
U 1 1 5AFA1462
P 2700 1400
F 0 "C15" V 2550 1400 50  0000 C CNN
F 1 "100n" V 2850 1400 50  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 2738 1250 50  0001 C CNN
F 3 "~" H 2700 1400 50  0001 C CNN
	1    2700 1400
	1    0    0    -1  
$EndComp
Wire Wire Line
	2300 550  2700 550 
Wire Wire Line
	2700 550  2700 650 
Connection ~ 2300 550 
Wire Wire Line
	2300 1050 2700 1050
Wire Wire Line
	2700 1050 2700 950 
Connection ~ 2300 1050
$Comp
L MainBoard-rescue:C-Device-MainBoard-rescue C4
U 1 1 5AFAE2C6
P 2300 800
F 0 "C4" V 2150 800 50  0000 C CNN
F 1 "100n" V 2450 800 50  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 2338 650 50  0001 C CNN
F 3 "~" H 2300 800 50  0001 C CNN
	1    2300 800 
	1    0    0    -1  
$EndComp
$Comp
L MainBoard-rescue:C-Device-MainBoard-rescue C7
U 1 1 5AFAE30E
P 3500 800
F 0 "C7" V 3350 800 50  0000 C CNN
F 1 "100n" V 3650 800 50  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 3538 650 50  0001 C CNN
F 3 "~" H 3500 800 50  0001 C CNN
	1    3500 800 
	1    0    0    -1  
$EndComp
$Comp
L MainBoard-rescue:C-Device-MainBoard-rescue C2
U 1 1 5AFAE35E
P 1500 800
F 0 "C2" V 1350 800 50  0000 C CNN
F 1 "4.7u" V 1650 800 50  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 1538 650 50  0001 C CNN
F 3 "~" H 1500 800 50  0001 C CNN
	1    1500 800 
	1    0    0    -1  
$EndComp
Wire Wire Line
	3500 550  3500 650 
Connection ~ 2700 550 
Wire Wire Line
	3500 950  3500 1050
Connection ~ 2700 1050
Wire Wire Line
	3100 950  3100 1050
Connection ~ 3100 1050
Wire Wire Line
	3100 1050 2700 1050
Wire Wire Line
	3100 650  3100 550 
Wire Wire Line
	2700 550  3100 550 
Connection ~ 3100 550 
Wire Wire Line
	3100 550  3500 550 
Wire Wire Line
	9700 5450 9800 5450
Wire Wire Line
	9700 5350 9800 5350
Wire Wire Line
	8500 5300 8400 5300
Wire Wire Line
	8500 5400 8400 5400
$Comp
L MainBoard-rescue:+5V-power-MainBoard-rescue #PWR045
U 1 1 5AF3CCEF
P 8400 5300
F 0 "#PWR045" H 8400 5150 50  0001 C CNN
F 1 "+5V" V 8400 5450 50  0000 L CNN
F 2 "" H 8400 5300 50  0001 C CNN
F 3 "" H 8400 5300 50  0001 C CNN
	1    8400 5300
	0    -1   -1   0   
$EndComp
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR047
U 1 1 5AF3CE80
P 8400 5400
F 0 "#PWR047" H 8400 5150 50  0001 C CNN
F 1 "GND" V 8400 5250 50  0000 R CNN
F 2 "" H 8400 5400 50  0001 C CNN
F 3 "" H 8400 5400 50  0001 C CNN
	1    8400 5400
	0    1    1    0   
$EndComp
$Comp
L MainBoard-rescue:+3.3V-power-MainBoard-rescue #PWR046
U 1 1 5AF3CEC7
P 9800 5350
F 0 "#PWR046" H 9800 5200 50  0001 C CNN
F 1 "+3.3V" V 9800 5500 50  0000 L CNN
F 2 "" H 9800 5350 50  0001 C CNN
F 3 "" H 9800 5350 50  0001 C CNN
	1    9800 5350
	0    1    1    0   
$EndComp
$Comp
L MainBoard-rescue:+3.3V-power-MainBoard-rescue #PWR048
U 1 1 5AF3CF2C
P 9800 5450
F 0 "#PWR048" H 9800 5300 50  0001 C CNN
F 1 "+3.3V" V 9800 5600 50  0000 L CNN
F 2 "" H 9800 5450 50  0001 C CNN
F 3 "" H 9800 5450 50  0001 C CNN
	1    9800 5450
	0    1    1    0   
$EndComp
Wire Wire Line
	8500 5600 8400 5600
$Comp
L MainBoard-rescue:+5V-power-MainBoard-rescue #PWR049
U 1 1 5AF44955
P 8400 5600
F 0 "#PWR049" H 8400 5450 50  0001 C CNN
F 1 "+5V" V 8400 5750 50  0000 L CNN
F 2 "" H 8400 5600 50  0001 C CNN
F 3 "" H 8400 5600 50  0001 C CNN
	1    8400 5600
	0    -1   -1   0   
$EndComp
Wire Wire Line
	8500 5700 8400 5700
Wire Wire Line
	8400 5700 8400 5800
$Comp
L MainBoard-rescue:C-Device-MainBoard-rescue C21
U 1 1 5AF4C54C
P 8400 5950
F 0 "C21" V 8250 5950 50  0000 C CNN
F 1 "10n" V 8550 5950 50  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 8438 5800 50  0001 C CNN
F 3 "~" H 8400 5950 50  0001 C CNN
	1    8400 5950
	1    0    0    -1  
$EndComp
Wire Wire Line
	8400 6100 8400 6200
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR051
U 1 1 5AF543CA
P 8400 6200
F 0 "#PWR051" H 8400 5950 50  0001 C CNN
F 1 "GND" H 8400 6050 50  0000 C CNN
F 2 "" H 8400 6200 50  0001 C CNN
F 3 "" H 8400 6200 50  0001 C CNN
	1    8400 6200
	1    0    0    -1  
$EndComp
NoConn ~ 9700 5650
$Comp
L MainBoard:LP2989 U8
U 1 1 5AF104C7
P 9100 5500
F 0 "U8" H 9100 5965 50  0000 C CNN
F 1 "LP2989" H 9100 5874 50  0000 C CNN
F 2 "MainBoard:SOIC-8" H 9100 5150 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/lp2989.pdf" H 9100 5500 50  0001 C CNN
	1    9100 5500
	1    0    0    -1  
$EndComp
$Comp
L MainBoard-rescue:C-Device-MainBoard-rescue C1
U 1 1 5AF6C3F6
P 1100 800
F 0 "C1" V 950 800 50  0000 C CNN
F 1 "47u" V 1250 800 50  0000 C CNN
F 2 "Capacitor_SMD:C_1206_3216Metric" H 1138 650 50  0001 C CNN
F 3 "~" H 1100 800 50  0001 C CNN
	1    1100 800 
	1    0    0    -1  
$EndComp
Wire Wire Line
	3500 550  3900 550 
Wire Wire Line
	3900 550  3900 650 
Connection ~ 3500 550 
Wire Wire Line
	3900 1050 3900 950 
Wire Wire Line
	3100 1050 3500 1050
Connection ~ 3500 1050
Wire Wire Line
	3500 1050 3900 1050
$Comp
L MainBoard-rescue:C-Device-MainBoard-rescue C12
U 1 1 5AF7C7B6
P 1100 1400
F 0 "C12" V 950 1400 50  0000 C CNN
F 1 "4.7u" V 1250 1400 50  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 1138 1250 50  0001 C CNN
F 3 "~" H 1100 1400 50  0001 C CNN
	1    1100 1400
	1    0    0    -1  
$EndComp
$Comp
L MainBoard-rescue:+3.3V-power-MainBoard-rescue #PWR037
U 1 1 5AF7D667
P 5900 4800
F 0 "#PWR037" H 5900 4650 50  0001 C CNN
F 1 "+3.3V" V 5900 4950 50  0000 L CNN
F 2 "" H 5900 4800 50  0001 C CNN
F 3 "" H 5900 4800 50  0001 C CNN
	1    5900 4800
	0    1    1    0   
$EndComp
$Comp
L MainBoard-rescue:C-Device-MainBoard-rescue C5
U 1 1 5AF7E047
P 2700 800
F 0 "C5" V 2550 800 50  0000 C CNN
F 1 "1u" V 2850 800 50  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 2738 650 50  0001 C CNN
F 3 "~" H 2700 800 50  0001 C CNN
	1    2700 800 
	1    0    0    -1  
$EndComp
$Comp
L MainBoard-rescue:C-Device-MainBoard-rescue C9
U 1 1 5AF7E0F5
P 4300 800
F 0 "C9" V 4150 800 50  0000 C CNN
F 1 "10n" V 4450 800 50  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 4338 650 50  0001 C CNN
F 3 "~" H 4300 800 50  0001 C CNN
	1    4300 800 
	1    0    0    -1  
$EndComp
$Comp
L MainBoard-rescue:C-Device-MainBoard-rescue C10
U 1 1 5AF7F2B6
P 4700 800
F 0 "C10" V 4550 800 50  0000 C CNN
F 1 "10n" V 4850 800 50  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 4738 650 50  0001 C CNN
F 3 "~" H 4700 800 50  0001 C CNN
	1    4700 800 
	1    0    0    -1  
$EndComp
Wire Wire Line
	3900 550  4300 550 
Wire Wire Line
	4300 550  4300 650 
Connection ~ 3900 550 
Wire Wire Line
	3900 1050 4300 1050
Wire Wire Line
	4300 1050 4300 950 
Connection ~ 3900 1050
Wire Wire Line
	4700 950  4700 1050
Wire Wire Line
	4700 1050 4300 1050
Connection ~ 4300 1050
Wire Wire Line
	4300 550  4700 550 
Wire Wire Line
	4700 550  4700 650 
Connection ~ 4300 550 
$Comp
L MainBoard-rescue:C-Device-MainBoard-rescue C16
U 1 1 5AFA1F1A
P 3100 1400
F 0 "C16" V 2950 1400 50  0000 C CNN
F 1 "10u" V 3250 1400 50  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 3138 1250 50  0001 C CNN
F 3 "~" H 3100 1400 50  0001 C CNN
	1    3100 1400
	1    0    0    -1  
$EndComp
$Comp
L MainBoard-rescue:C-Device-MainBoard-rescue C17
U 1 1 5AFA1F74
P 1900 1400
F 0 "C17" V 1750 1400 50  0000 C CNN
F 1 "100n" V 2050 1400 50  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 1938 1250 50  0001 C CNN
F 3 "~" H 1900 1400 50  0001 C CNN
	1    1900 1400
	1    0    0    -1  
$EndComp
Wire Wire Line
	2300 1650 2300 1550
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR040
U 1 1 5AFF0D54
P 4500 5000
F 0 "#PWR040" H 4500 4750 50  0001 C CNN
F 1 "GND" V 4500 4850 50  0000 R CNN
F 2 "" H 4500 5000 50  0001 C CNN
F 3 "" H 4500 5000 50  0001 C CNN
	1    4500 5000
	0    1    1    0   
$EndComp
Text Label 4300 3850 0    50   ~ 0
SDA
Wire Wire Line
	4300 3750 4600 3750
Wire Wire Line
	4300 3850 4600 3850
Wire Wire Line
	2300 2250 2500 2250
Wire Wire Line
	2300 2350 2500 2350
Wire Wire Line
	2300 2450 2500 2450
Text Label 2500 2250 2    50   ~ 0
SDA
Text Label 2500 2350 2    50   ~ 0
SCL
Text Label 2500 2450 2    50   ~ 0
WC
Wire Wire Line
	2300 3050 2500 3050
Wire Wire Line
	2300 3150 2500 3150
Wire Wire Line
	2300 3250 2500 3250
Text Label 2500 3050 2    50   ~ 0
SDA
Text Label 2500 3150 2    50   ~ 0
SCL
Text Label 2500 3250 2    50   ~ 0
WC
Wire Wire Line
	4600 2950 4300 2950
Wire Wire Line
	4600 3050 4300 3050
Text Label 4300 2950 0    50   ~ 0
SWDIO
Text Label 4300 3050 0    50   ~ 0
SWCLK
Wire Wire Line
	4400 4700 4600 4700
Text Label 4400 4700 0    50   ~ 0
RST
$Comp
L Connector_Generic:Conn_01x06 J2
U 1 1 5B04F48B
P 9450 2900
F 0 "J2" H 9450 3200 50  0000 C CNN
F 1 "SWDebug" V 9550 2900 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x06_P2.54mm_Vertical" H 9450 2900 50  0001 C CNN
F 3 "~" H 9450 2900 50  0001 C CNN
	1    9450 2900
	1    0    0    -1  
$EndComp
Wire Wire Line
	9250 2800 9150 2800
$Comp
L MainBoard-rescue:+3.3V-power-MainBoard-rescue #PWR021
U 1 1 5B059441
P 9150 2800
F 0 "#PWR021" H 9150 2650 50  0001 C CNN
F 1 "+3.3V" V 9150 2950 50  0000 L CNN
F 2 "" H 9150 2800 50  0001 C CNN
F 3 "" H 9150 2800 50  0001 C CNN
	1    9150 2800
	0    -1   -1   0   
$EndComp
Wire Wire Line
	9250 3200 9150 3200
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR024
U 1 1 5B0816C3
P 9150 3200
F 0 "#PWR024" H 9150 2950 50  0001 C CNN
F 1 "GND" V 9150 3050 50  0000 R CNN
F 2 "" H 9150 3200 50  0001 C CNN
F 3 "" H 9150 3200 50  0001 C CNN
	1    9150 3200
	0    1    1    0   
$EndComp
Text Label 8950 2900 0    50   ~ 0
SWDIO
Text Label 8950 3000 0    50   ~ 0
SWCLK
Wire Wire Line
	8950 2900 9250 2900
Wire Wire Line
	8950 3000 9250 3000
Wire Notes Line
	8050 4900 8050 6400
Wire Notes Line
	8050 6400 10200 6400
Wire Notes Line
	10200 6400 10200 4900
Wire Notes Line
	10200 4900 8050 4900
Text Notes 8050 4900 0    50   ~ 0
3.3V VOLTAGE REGULATOR
Wire Wire Line
	4300 3250 4600 3250
Text Label 4300 3250 0    50   ~ 0
WC
$Comp
L MainBoard-rescue:+5V-power-MainBoard-rescue #PWR010
U 1 1 5B0CCA56
P 4500 1400
F 0 "#PWR010" H 4500 1250 50  0001 C CNN
F 1 "+5V" V 4500 1550 50  0000 L CNN
F 2 "" H 4500 1400 50  0001 C CNN
F 3 "" H 4500 1400 50  0001 C CNN
	1    4500 1400
	0    -1   1    0   
$EndComp
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR013
U 1 1 5B0CCAAF
P 4500 1500
F 0 "#PWR013" H 4500 1250 50  0001 C CNN
F 1 "GND" V 4500 1350 50  0000 R CNN
F 2 "" H 4500 1500 50  0001 C CNN
F 3 "" H 4500 1500 50  0001 C CNN
	1    4500 1500
	0    1    1    0   
$EndComp
$Comp
L MainBoard-rescue:PWR_FLAG-power-MainBoard-rescue #FLG02
U 1 1 5B0CCBD7
P 4600 1400
F 0 "#FLG02" H 4600 1475 50  0001 C CNN
F 1 "PWR_FLAG" V 4600 1550 50  0000 L CNN
F 2 "" H 4600 1400 50  0001 C CNN
F 3 "" H 4600 1400 50  0001 C CNN
	1    4600 1400
	0    1    1    0   
$EndComp
$Comp
L MainBoard-rescue:PWR_FLAG-power-MainBoard-rescue #FLG03
U 1 1 5B0CCD5C
P 4600 1500
F 0 "#FLG03" H 4600 1575 50  0001 C CNN
F 1 "PWR_FLAG" V 4600 1650 50  0000 L CNN
F 2 "" H 4600 1500 50  0001 C CNN
F 3 "" H 4600 1500 50  0001 C CNN
	1    4600 1500
	0    1    1    0   
$EndComp
Wire Wire Line
	4500 1400 4600 1400
Wire Wire Line
	4500 1500 4600 1500
Wire Notes Line
	9700 2500 8700 2500
Text Notes 8700 2500 0    50   ~ 0
SWD INTERFACE
$Comp
L MainBoard-rescue:C-Device-MainBoard-rescue C11
U 1 1 5B39F402
P 5100 800
F 0 "C11" V 4950 800 50  0000 C CNN
F 1 "100n" V 5250 800 50  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 5138 650 50  0001 C CNN
F 3 "~" H 5100 800 50  0001 C CNN
	1    5100 800 
	1    0    0    -1  
$EndComp
Wire Wire Line
	4700 550  5100 550 
Wire Wire Line
	5100 550  5100 650 
Connection ~ 4700 550 
Wire Wire Line
	5100 1050 4700 1050
Wire Wire Line
	5100 950  5100 1050
Connection ~ 4700 1050
$Comp
L Connector_Generic:Conn_01x08 J3
U 1 1 5B191837
P 2000 5200
F 0 "J3" H 2000 5600 50  0000 C CNN
F 1 "IMD" V 2100 5150 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x08_P2.54mm_Vertical" H 2000 5200 50  0001 C CNN
F 3 "https://www.bender.de/fileadmin/content/Products/d/e/IR155-32xx-V004_D00115_D_XXEN.pdf" H 2000 5200 50  0001 C CNN
	1    2000 5200
	1    0    0    -1  
$EndComp
Wire Wire Line
	1800 4900 1700 4900
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR038
U 1 1 5B1CEF4A
P 1700 4900
F 0 "#PWR038" H 1700 4650 50  0001 C CNN
F 1 "GND" V 1700 4750 50  0000 R CNN
F 2 "" H 1700 4900 50  0001 C CNN
F 3 "" H 1700 4900 50  0001 C CNN
	1    1700 4900
	0    1    1    0   
$EndComp
Wire Wire Line
	1800 5100 1700 5100
Wire Wire Line
	1800 5200 1700 5200
Wire Wire Line
	1700 5000 1800 5000
NoConn ~ 1800 5500
Text Label 1400 5600 0    50   ~ 0
IMD_Fault
Wire Wire Line
	1400 5600 1800 5600
NoConn ~ 1800 5400
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR044
U 1 1 5B541514
P 1700 5200
F 0 "#PWR044" H 1700 4950 50  0001 C CNN
F 1 "GND" V 1700 5050 50  0000 R CNN
F 2 "" H 1700 5200 50  0001 C CNN
F 3 "" H 1700 5200 50  0001 C CNN
	1    1700 5200
	0    1    1    0   
$EndComp
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR042
U 1 1 5B54160F
P 1700 5100
F 0 "#PWR042" H 1700 4850 50  0001 C CNN
F 1 "GND" V 1700 4950 50  0000 R CNN
F 2 "" H 1700 5100 50  0001 C CNN
F 3 "" H 1700 5100 50  0001 C CNN
	1    1700 5100
	0    1    1    0   
$EndComp
$Comp
L MainBoard-rescue:+12V-power-MainBoard-rescue #PWR039
U 1 1 5B541F7A
P 1700 5000
F 0 "#PWR039" H 1700 4850 50  0001 C CNN
F 1 "+12V" V 1700 5150 50  0000 L CNN
F 2 "" H 1700 5000 50  0001 C CNN
F 3 "" H 1700 5000 50  0001 C CNN
	1    1700 5000
	0    -1   -1   0   
$EndComp
Text GLabel 1950 6250 2    50   Input ~ 0
BMS_LED
Text GLabel 1950 6350 2    50   Input ~ 0
To_TSMS
Text GLabel 1250 6350 0    50   Input ~ 0
RESET_BUTTON
Text GLabel 1950 7150 2    50   Input ~ 0
From_TSMS
Wire Wire Line
	3100 6300 2700 6300
Wire Wire Line
	3100 6400 2700 6400
Wire Wire Line
	3100 6500 2700 6500
Wire Wire Line
	3100 6600 2700 6600
Text Label 2700 6500 0    50   ~ 0
IMD_Fault
Text Label 2700 6600 0    50   ~ 0
BMS_Fault
Text Label 2700 6400 0    50   ~ 0
TS_ON
Text Label 2700 6300 0    50   ~ 0
PC_Ended
Text Label 6200 3000 2    50   ~ 0
PC_Ended
Wire Wire Line
	5800 3000 6200 3000
Wire Wire Line
	5800 3800 6200 3800
Wire Wire Line
	5800 3900 6200 3900
Text Label 6200 3800 2    50   ~ 0
TS_ON
Text Label 6200 3900 2    50   ~ 0
BMS_Fault
Wire Wire Line
	5800 3700 6200 3700
Text Label 4300 3150 0    50   ~ 0
IMD
Wire Wire Line
	1000 5300 1000 5400
$Comp
L MainBoard-rescue:R-Device-MainBoard-rescue R10
U 1 1 5B4093B7
P 1000 5550
F 0 "R10" V 900 5550 50  0000 C CNN
F 1 "590" V 1000 5550 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 930 5550 50  0001 C CNN
F 3 "~" H 1000 5550 50  0001 C CNN
	1    1000 5550
	1    0    0    -1  
$EndComp
Wire Wire Line
	1000 5700 1000 5800
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR050
U 1 1 5B42E62A
P 1000 5800
F 0 "#PWR050" H 1000 5550 50  0001 C CNN
F 1 "GND" H 1000 5650 50  0000 C CNN
F 2 "" H 1000 5800 50  0001 C CNN
F 3 "" H 1000 5800 50  0001 C CNN
	1    1000 5800
	1    0    0    -1  
$EndComp
$Comp
L MainBoard-rescue:R-Device-MainBoard-rescue R9
U 1 1 5B45397A
P 1250 5300
F 0 "R9" V 1150 5300 50  0000 C CNN
F 1 "1.6K" V 1250 5300 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 1180 5300 50  0001 C CNN
F 3 "~" H 1250 5300 50  0001 C CNN
	1    1250 5300
	0    1    1    0   
$EndComp
Wire Wire Line
	1000 5300 1100 5300
Wire Wire Line
	1400 5300 1800 5300
Wire Wire Line
	1000 5300 800  5300
Connection ~ 1000 5300
Text Label 800  5300 0    50   ~ 0
IMD
$Sheet
S 3100 5200 900  700 
U 5B50ED38
F0 "HV" 50
F1 "HVSide.sch" 50
F2 "MOSI" I L 3100 5400 50 
F3 "CS" I L 3100 5500 50 
F4 "SCK" I L 3100 5600 50 
F5 "MISO" I L 3100 5700 50 
$EndSheet
Wire Wire Line
	2800 5400 3100 5400
Wire Wire Line
	3100 5500 2800 5500
Wire Wire Line
	3100 5600 2800 5600
Wire Wire Line
	3100 5700 2800 5700
Text Label 2800 5400 0    50   ~ 0
MOSI
Text Label 2800 5500 0    50   ~ 0
PADC
Text Label 2800 5600 0    50   ~ 0
SCK
Text Label 2800 5700 0    50   ~ 0
MISO
Text Label 6200 3100 2    50   ~ 0
PADC
Wire Wire Line
	5800 3100 6200 3100
NoConn ~ 4600 3350
NoConn ~ 4600 3950
NoConn ~ 4600 4050
NoConn ~ 5800 3600
Wire Wire Line
	5800 3200 6200 3200
Text Label 6200 3200 2    50   ~ 0
6820
Text Label 8000 1350 2    50   ~ 0
6820
Text GLabel 1950 7250 2    50   Input ~ 0
RelayStat
Text GLabel 1250 7250 0    50   Input ~ 0
NRelayStat
$Comp
L power:+12V #PWR08
U 1 1 5B9FF1E1
P 4500 1300
F 0 "#PWR08" H 4500 1150 50  0001 C CNN
F 1 "+12V" V 4500 1450 50  0000 L CNN
F 2 "" H 4500 1300 50  0001 C CNN
F 3 "" H 4500 1300 50  0001 C CNN
	1    4500 1300
	0    -1   -1   0   
$EndComp
Wire Wire Line
	4500 1300 4600 1300
$Comp
L MainBoard-rescue:PWR_FLAG-power-MainBoard-rescue #FLG01
U 1 1 5BA0DF46
P 4600 1300
F 0 "#FLG01" H 4600 1375 50  0001 C CNN
F 1 "PWR_FLAG" V 4600 1450 50  0000 L CNN
F 2 "" H 4600 1300 50  0001 C CNN
F 3 "" H 4600 1300 50  0001 C CNN
	1    4600 1300
	0    1    1    0   
$EndComp
$Comp
L power:+12V #PWR053
U 1 1 5BA2D579
P 2250 6550
F 0 "#PWR053" H 2250 6400 50  0001 C CNN
F 1 "+12V" V 2250 6700 50  0000 L CNN
F 2 "" H 2250 6550 50  0001 C CNN
F 3 "" H 2250 6550 50  0001 C CNN
	1    2250 6550
	0    1    1    0   
$EndComp
$Comp
L MainBoard-rescue:+5V-power-MainBoard-rescue #PWR054
U 1 1 5BA3CD38
P 950 6550
F 0 "#PWR054" H 950 6400 50  0001 C CNN
F 1 "+5V" V 950 6700 50  0000 L CNN
F 2 "" H 950 6550 50  0001 C CNN
F 3 "" H 950 6550 50  0001 C CNN
	1    950  6550
	0    -1   -1   0   
$EndComp
Text Label 2050 7050 2    50   ~ 0
CAN+
Text Label 1150 6950 0    50   ~ 0
CAN-
Text GLabel 1250 6450 0    50   Input ~ 0
From_SD
Text GLabel 1950 6450 2    50   Input ~ 0
INV_Enable
Text GLabel 1250 6250 0    50   Input ~ 0
IMD_LED
Wire Wire Line
	5800 3300 6200 3300
Text Label 6200 3300 2    50   ~ 0
SDCard
Text Label 2600 4050 2    50   ~ 0
SDCard
Wire Wire Line
	2300 4050 2600 4050
Wire Wire Line
	2300 3750 2600 3750
Wire Wire Line
	2300 3850 2600 3850
Wire Wire Line
	2300 3950 2600 3950
Wire Wire Line
	4600 3150 4300 3150
Text Label 6200 3700 2    50   ~ 0
CANS
Text Label 2050 6950 2    50   ~ 0
CANS
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR060
U 1 1 5BC6DE63
P 1250 7050
F 0 "#PWR060" H 1250 6800 50  0001 C CNN
F 1 "GND" V 1250 6900 50  0000 R CNN
F 2 "" H 1250 7050 50  0001 C CNN
F 3 "" H 1250 7050 50  0001 C CNN
	1    1250 7050
	0    1    1    0   
$EndComp
$Comp
L MainBoard:REF2920 U10
U 1 1 5B042543
P 6100 6700
F 0 "U10" H 6100 7015 50  0000 C CNN
F 1 "REF2920" H 6100 6924 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 6100 6500 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/ref2920.pdf" H 6100 6700 50  0001 C CNN
	1    6100 6700
	-1   0    0    -1  
$EndComp
Wire Wire Line
	6400 6650 6500 6650
Wire Wire Line
	6400 6750 6500 6750
$Comp
L MainBoard-rescue:+5V-power-MainBoard-rescue #PWR056
U 1 1 5B08E357
P 6500 6650
F 0 "#PWR056" H 6500 6500 50  0001 C CNN
F 1 "+5V" V 6500 6800 50  0000 L CNN
F 2 "" H 6500 6650 50  0001 C CNN
F 3 "" H 6500 6650 50  0001 C CNN
	1    6500 6650
	0    1    1    0   
$EndComp
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR057
U 1 1 5B08E4AA
P 6500 6750
F 0 "#PWR057" H 6500 6500 50  0001 C CNN
F 1 "GND" V 6500 6600 50  0000 R CNN
F 2 "" H 6500 6750 50  0001 C CNN
F 3 "" H 6500 6750 50  0001 C CNN
	1    6500 6750
	0    -1   1    0   
$EndComp
$Comp
L MainBoard:HTFS200-P U9
U 1 1 5B0EDC8D
P 5100 6550
F 0 "U9" H 5100 6965 50  0000 C CNN
F 1 "HTFS200-P" H 5100 6874 50  0000 C CNN
F 2 "MainBoard:HTFS 200-P" H 5100 6250 50  0001 C CNN
F 3 "https://www.lem.com/sites/default/files/products_datasheets/htfs_200_800-p.pdf" H 5100 6550 50  0001 C CNN
	1    5100 6550
	1    0    0    -1  
$EndComp
Wire Wire Line
	4600 6450 4500 6450
Wire Wire Line
	4600 6650 4500 6650
Wire Wire Line
	5600 6650 5700 6650
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR055
U 1 1 5B13B511
P 4500 6650
F 0 "#PWR055" H 4500 6400 50  0001 C CNN
F 1 "GND" V 4500 6500 50  0000 R CNN
F 2 "" H 4500 6650 50  0001 C CNN
F 3 "" H 4500 6650 50  0001 C CNN
	1    4500 6650
	0    1    1    0   
$EndComp
Wire Wire Line
	5600 6450 5900 6450
Text Label 5900 6450 2    50   ~ 0
Current
Wire Wire Line
	5700 6650 5700 6750
Connection ~ 5700 6650
Wire Wire Line
	5700 6650 5800 6650
$Comp
L MainBoard-rescue:C-Device-MainBoard-rescue C22
U 1 1 5B163079
P 5700 6900
F 0 "C22" V 5550 6900 50  0000 C CNN
F 1 "47n" V 5850 6900 50  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 5738 6750 50  0001 C CNN
F 3 "~" H 5700 6900 50  0001 C CNN
	1    5700 6900
	1    0    0    -1  
$EndComp
Wire Wire Line
	5700 7050 5700 7150
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR058
U 1 1 5B176DFE
P 5700 7150
F 0 "#PWR058" H 5700 6900 50  0001 C CNN
F 1 "GND" H 5700 7000 50  0000 C CNN
F 2 "" H 5700 7150 50  0001 C CNN
F 3 "" H 5700 7150 50  0001 C CNN
	1    5700 7150
	1    0    0    -1  
$EndComp
$Comp
L MainBoard-rescue:+5V-power-MainBoard-rescue #PWR052
U 1 1 5B177068
P 4500 6450
F 0 "#PWR052" H 4500 6300 50  0001 C CNN
F 1 "+5V" V 4500 6600 50  0000 L CNN
F 2 "" H 4500 6450 50  0001 C CNN
F 3 "" H 4500 6450 50  0001 C CNN
	1    4500 6450
	0    -1   -1   0   
$EndComp
$Comp
L MainBoard-rescue:C-Device-MainBoard-rescue C18
U 1 1 5B18AF8F
P 2300 1400
F 0 "C18" V 2150 1400 50  0000 C CNN
F 1 "47n" V 2450 1400 50  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 2338 1250 50  0001 C CNN
F 3 "~" H 2300 1400 50  0001 C CNN
	1    2300 1400
	1    0    0    -1  
$EndComp
Wire Wire Line
	2300 1150 2300 1250
Wire Wire Line
	6900 2900 6900 3000
$Comp
L MainBoard-rescue:C-Device-MainBoard-rescue C20
U 1 1 5B22FE04
P 6900 3150
F 0 "C20" V 6750 3150 50  0000 C CNN
F 1 "4.7n" V 7050 3150 50  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 6938 3000 50  0001 C CNN
F 3 "~" H 6900 3150 50  0001 C CNN
	1    6900 3150
	1    0    0    -1  
$EndComp
Wire Wire Line
	6900 3300 6900 3400
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR027
U 1 1 5B244D70
P 6900 3400
F 0 "#PWR027" H 6900 3150 50  0001 C CNN
F 1 "GND" H 6900 3250 50  0000 C CNN
F 2 "" H 6900 3400 50  0001 C CNN
F 3 "" H 6900 3400 50  0001 C CNN
	1    6900 3400
	1    0    0    -1  
$EndComp
Wire Wire Line
	6900 2900 7200 2900
Connection ~ 6900 2900
Text Label 7200 2900 2    50   ~ 0
Current
Wire Notes Line
	4100 6000 4100 7350
Wire Notes Line
	4100 7350 6900 7350
Wire Notes Line
	6900 7350 6900 6000
Wire Notes Line
	6900 6000 4100 6000
Text Notes 4100 6000 0    50   ~ 0
CURRENT SENSOR
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR033
U 1 1 5B211A61
P 1000 4050
F 0 "#PWR033" H 1000 3800 50  0001 C CNN
F 1 "GND" V 1000 3900 50  0000 R CNN
F 2 "" H 1000 4050 50  0001 C CNN
F 3 "" H 1000 4050 50  0001 C CNN
	1    1000 4050
	0    1    1    0   
$EndComp
Wire Wire Line
	1000 4050 1100 4050
Text Label 1000 3950 0    50   ~ 0
CD
Wire Wire Line
	1000 3950 1100 3950
Wire Wire Line
	5800 3400 6200 3400
Text Label 6200 3400 2    50   ~ 0
CD
Wire Wire Line
	5800 3500 6200 3500
Text Label 6200 3500 2    50   ~ 0
SD_Status
$Sheet
S 3100 6100 900  800 
U 5B045B0D
F0 "Shutdown" 50
F1 "MainShutdown.sch" 50
F2 "PC_Ended" I L 3100 6300 50 
F3 "TS_ON" I L 3100 6400 50 
F4 "IMD_Status" I L 3100 6500 50 
F5 "BMS_Status" I L 3100 6600 50 
F6 "SD_Status" I L 3100 6700 50 
$EndSheet
Wire Wire Line
	3100 6700 2700 6700
Text Label 2700 6700 0    50   ~ 0
SD_Status
Wire Wire Line
	7800 1050 8000 1050
Wire Wire Line
	4100 3550 4600 3550
Wire Wire Line
	4100 3550 4100 3450
$Comp
L MainBoard-rescue:R-Device-MainBoard-rescue R51
U 1 1 5B2A353A
P 4100 3300
F 0 "R51" V 4200 3300 50  0000 C CNN
F 1 "1K" V 4100 3300 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 4030 3300 50  0001 C CNN
F 3 "~" H 4100 3300 50  0001 C CNN
	1    4100 3300
	-1   0    0    1   
$EndComp
Wire Wire Line
	4100 3150 4100 3050
$Comp
L MainBoard-rescue:+3.3V-power-MainBoard-rescue #PWR0141
U 1 1 5B2B9A60
P 4100 3050
F 0 "#PWR0141" H 4100 2900 50  0001 C CNN
F 1 "+3.3V" H 4100 3200 50  0000 C CNN
F 2 "" H 4100 3050 50  0001 C CNN
F 3 "" H 4100 3050 50  0001 C CNN
	1    4100 3050
	1    0    0    -1  
$EndComp
NoConn ~ 8350 1700
Wire Wire Line
	2300 1650 2700 1650
Wire Wire Line
	2700 1650 3100 1650
Connection ~ 2700 1650
Wire Wire Line
	3100 1250 3100 1150
Wire Wire Line
	2700 1250 2700 1150
Wire Wire Line
	1500 1550 1500 1650
Connection ~ 1900 1650
Wire Wire Line
	1900 1650 2300 1650
Wire Wire Line
	1900 1550 1900 1650
Wire Wire Line
	1100 1650 1500 1650
Wire Wire Line
	1100 1150 1500 1150
Connection ~ 1500 1150
Wire Wire Line
	1500 1150 1900 1150
Connection ~ 1500 1650
Wire Wire Line
	1500 1650 1900 1650
$Comp
L Device:Polyfuse_Small F2
U 1 1 5B2F3DA2
P 2050 6550
F 0 "F2" V 1845 6550 50  0000 C CNN
F 1 "0ZCG 2A" V 1936 6550 50  0000 C CNN
F 2 "Fuse:Fuse_1812_4532Metric" H 2100 6350 50  0001 L CNN
F 3 "~" H 2050 6550 50  0001 C CNN
	1    2050 6550
	0    -1   -1   0   
$EndComp
$Comp
L Device:Polyfuse_Small F3
U 1 1 5B3092A1
P 1150 6550
F 0 "F3" V 945 6550 50  0000 C CNN
F 1 "0ZCG 2A" V 1036 6550 50  0000 C CNN
F 2 "Fuse:Fuse_1812_4532Metric" H 1200 6350 50  0001 L CNN
F 3 "~" H 1150 6550 50  0001 C CNN
	1    1150 6550
	0    -1   -1   0   
$EndComp
$Comp
L Switch:SW_Push SW1
U 1 1 5B62B580
P 4200 4700
F 0 "SW1" H 4200 4985 50  0000 C CNN
F 1 "RESET" H 4200 4894 50  0000 C CNN
F 2 "Button_Switch_SMD:SW_SPST_B3U-1000P" H 4200 4700 50  0001 C CNN
F 3 "" H 4200 4700 50  0001 C CNN
	1    4200 4700
	-1   0    0    -1  
$EndComp
Wire Wire Line
	4000 4700 3900 4700
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR0142
U 1 1 5B640CF2
P 3900 4700
F 0 "#PWR0142" H 3900 4450 50  0001 C CNN
F 1 "GND" V 3900 4550 50  0000 R CNN
F 2 "" H 3900 4700 50  0001 C CNN
F 3 "" H 3900 4700 50  0001 C CNN
	1    3900 4700
	0    1    1    0   
$EndComp
Connection ~ 2300 1650
Wire Wire Line
	2300 1150 2700 1150
Connection ~ 2300 1150
Connection ~ 2700 1150
Wire Wire Line
	2700 1150 3100 1150
$Comp
L MainBoard-rescue:R-Device-MainBoard-rescue R52
U 1 1 5B52F900
P 10050 4000
F 0 "R52" V 10150 4000 50  0000 C CNN
F 1 "60" V 10050 4000 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 9980 4000 50  0001 C CNN
F 3 "~" H 10050 4000 50  0001 C CNN
	1    10050 4000
	-1   0    0    1   
$EndComp
Wire Wire Line
	10050 4250 10050 4200
Wire Wire Line
	10050 4200 10150 4200
Connection ~ 10050 4200
Wire Wire Line
	10050 4200 10050 4150
$Comp
L MainBoard-rescue:C-Device-MainBoard-rescue C13
U 1 1 5B55B5EF
P 10300 4200
F 0 "C13" V 10150 4200 50  0000 C CNN
F 1 "4.7n" V 10450 4200 50  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric" H 10338 4050 50  0001 C CNN
F 3 "~" H 10300 4200 50  0001 C CNN
	1    10300 4200
	0    1    1    0   
$EndComp
Wire Wire Line
	10450 4200 10550 4200
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR0155
U 1 1 5B5717F0
P 10550 4200
F 0 "#PWR0155" H 10550 3950 50  0001 C CNN
F 1 "GND" V 10550 4050 50  0000 R CNN
F 2 "" H 10550 4200 50  0001 C CNN
F 3 "" H 10550 4200 50  0001 C CNN
	1    10550 4200
	0    -1   -1   0   
$EndComp
Wire Wire Line
	10050 4550 10050 4600
Wire Wire Line
	10050 4600 9850 4600
Wire Wire Line
	9850 4600 9850 4250
Wire Wire Line
	9850 4150 9850 3800
Wire Wire Line
	9850 3800 10050 3800
Wire Wire Line
	10050 3800 10050 3850
Wire Wire Line
	9200 4250 9850 4250
Wire Wire Line
	9200 4150 9850 4150
Wire Wire Line
	9250 2700 9150 2700
$Comp
L MainBoard-rescue:+5V-power-MainBoard-rescue #PWR059
U 1 1 5B5F243A
P 9150 2700
F 0 "#PWR059" H 9150 2550 50  0001 C CNN
F 1 "+5V" V 9150 2850 50  0000 L CNN
F 2 "" H 9150 2700 50  0001 C CNN
F 3 "" H 9150 2700 50  0001 C CNN
	1    9150 2700
	0    -1   -1   0   
$EndComp
Wire Wire Line
	8300 1600 8350 1600
Wire Wire Line
	8300 1800 8350 1800
Wire Wire Line
	9550 1750 9650 1750
Wire Wire Line
	9550 1650 9650 1650
$Comp
L Connector_Generic:Conn_02x04_Top_Bottom J4
U 1 1 5B5ADC68
P 1650 6350
F 0 "J4" H 1700 6550 50  0000 C CNN
F 1 "IO1" H 1700 6050 50  0000 C CNN
F 2 "Connector_Molex:Molex_Micro-Fit_3.0_43045-0812_2x04_P3.00mm_Vertical" H 1650 6350 50  0001 C CNN
F 3 "~" H 1650 6350 50  0001 C CNN
	1    1650 6350
	-1   0    0    -1  
$EndComp
Wire Wire Line
	1850 6250 1950 6250
Wire Wire Line
	1850 6350 1950 6350
Wire Wire Line
	1850 6450 1950 6450
Wire Wire Line
	1850 6550 1950 6550
Wire Wire Line
	2150 6550 2250 6550
Wire Wire Line
	1250 6250 1350 6250
Wire Wire Line
	1250 6350 1350 6350
Wire Wire Line
	1250 6450 1350 6450
Wire Wire Line
	1250 6550 1350 6550
Wire Wire Line
	1050 6550 950  6550
$Comp
L Connector_Generic:Conn_02x04_Top_Bottom J11
U 1 1 5B6BCFDF
P 1650 7050
F 0 "J11" H 1700 7250 50  0000 C CNN
F 1 "IO2" H 1700 6750 50  0000 C CNN
F 2 "Connector_Molex:Molex_Micro-Fit_3.0_43045-0812_2x04_P3.00mm_Vertical" H 1650 7050 50  0001 C CNN
F 3 "~" H 1650 7050 50  0001 C CNN
	1    1650 7050
	-1   0    0    -1  
$EndComp
Wire Wire Line
	1850 6950 2050 6950
Wire Wire Line
	1850 7050 2050 7050
Wire Wire Line
	1950 7150 1850 7150
Wire Wire Line
	1850 7250 1950 7250
Wire Wire Line
	1150 6950 1350 6950
Wire Wire Line
	1250 7050 1350 7050
Wire Wire Line
	1250 7150 1350 7150
$Comp
L MainBoard-rescue:GND-power-MainBoard-rescue #PWR0156
U 1 1 5B777B6D
P 1250 7150
F 0 "#PWR0156" H 1250 6900 50  0001 C CNN
F 1 "GND" V 1250 7000 50  0000 R CNN
F 2 "" H 1250 7150 50  0001 C CNN
F 3 "" H 1250 7150 50  0001 C CNN
	1    1250 7150
	0    1    1    0   
$EndComp
Wire Wire Line
	1250 7250 1350 7250
Wire Wire Line
	5800 2900 6900 2900
Wire Wire Line
	8300 1650 8300 1600
Wire Wire Line
	8300 1800 8300 1750
Wire Wire Line
	9050 1800 9050 1900
Wire Wire Line
	9050 1900 9150 1900
Wire Wire Line
	9050 1600 9050 1500
Wire Wire Line
	9050 1500 9150 1500
Wire Wire Line
	9150 1900 9150 1850
Wire Wire Line
	9150 1500 9150 1550
Wire Wire Line
	9150 1500 9550 1500
Connection ~ 9150 1500
Wire Wire Line
	9150 1900 9550 1900
Connection ~ 9150 1900
Wire Wire Line
	9550 1500 9550 1650
Wire Wire Line
	9550 1750 9550 1900
Wire Wire Line
	9250 3100 8950 3100
Text Label 8950 3100 0    50   ~ 0
RST
Wire Notes Line
	8700 3300 9700 3300
Wire Notes Line
	8700 2500 8700 3300
Wire Notes Line
	9700 2500 9700 3300
$EndSCHEMATC