/*
THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF PARALLAX
SOFTWARE CORPORATION ("PARALLAX").  PARALLAX, IN DISTRIBUTING THE CODE TO
END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.
COPYRIGHT 1993-1998 PARALLAX SOFTWARE CORPORATION.  ALL RIGHTS RESERVED.
*/

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include "maths.h"

ubyte guess_table [] = {
	1,
	1,1,1,
	2,2,2,2,2,
	3,3,3,3,3,3,3,
	4,4,4,4,4,4,4,4,4,
	5,5,5,5,5,5,5,5,5,5,5,
	6,6,6,6,6,6,6,6,6,6,6,6,6,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
	9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
	11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,
	12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,
	13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,
	14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,
	15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15
};

short sincos_table [] = {
	0,
	402,
	804,
	1205,
	1606,
	2006,
	2404,
	2801,
	3196,
	3590,
	3981,
	4370,
	4756,
	5139,
	5520,
	5897,
	6270,
	6639,
	7005,
	7366,
	7723,
	8076,
	8423,
	8765,
	9102,
	9434,
	9760,
	10080,
	10394,
	10702,
	11003,
	11297,
	11585,
	11866,
	12140,
	12406,
	12665,
	12916,
	13160,
	13395,
	13623,
	13842,
	14053,
	14256,
	14449,
	14635,
	14811,
	14978,
	15137,
	15286,
	15426,
	15557,
	15679,
	15791,
	15893,
	15986,
	16069,
	16143,
	16207,
	16261,
	16305,
	16340,
	16364,
	16379,
	// cos_table
	16384,
	16379,
	16364,
	16340,
	16305,
	16261,
	16207,
	16143,
	16069,
	15986,
	15893,
	15791,
	15679,
	15557,
	15426,
	15286,
	15137,
	14978,
	14811,
	14635,
	14449,
	14256,
	14053,
	13842,
	13623,
	13395,
	13160,
	12916,
	12665,
	12406,
	12140,
	11866,
	11585,
	11297,
	11003,
	10702,
	10394,
	10080,
	9760,
	9434,
	9102,
	8765,
	8423,
	8076,
	7723,
	7366,
	7005,
	6639,
	6270,
	5897,
	5520,
	5139,
	4756,
	4370,
	3981,
	3590,
	3196,
	2801,
	2404,
	2006,
	1606,
	1205,
	804,
	402,
	0,
	-402,
	-804,
	-1205,
	-1606,
	-2006,
	-2404,
	-2801,
	-3196,
	-3590,
	-3981,
	-4370,
	-4756,
	-5139,
	-5520,
	-5897,
	-6270,
	-6639,
	-7005,
	-7366,
	-7723,
	-8076,
	-8423,
	-8765,
	-9102,
	-9434,
	-9760,
	-10080,
	-10394,
	-10702,
	-11003,
	-11297,
	-11585,
	-11866,
	-12140,
	-12406,
	-12665,
	-12916,
	-13160,
	-13395,
	-13623,
	-13842,
	-14053,
	-14256,
	-14449,
	-14635,
	-14811,
	-14978,
	-15137,
	-15286,
	-15426,
	-15557,
	-15679,
	-15791,
	-15893,
	-15986,
	-16069,
	-16143,
	-16207,
	-16261,
	-16305,
	-16340,
	-16364,
	-16379,
	-16384,
	-16379,
	-16364,
	-16340,
	-16305,
	-16261,
	-16207,
	-16143,
	-16069,
	-15986,
	-15893,
	-15791,
	-15679,
	-15557,
	-15426,
	-15286,
	-15137,
	-14978,
	-14811,
	-14635,
	-14449,
	-14256,
	-14053,
	-13842,
	-13623,
	-13395,
	-13160,
	-12916,
	-12665,
	-12406,
	-12140,
	-11866,
	-11585,
	-11297,
	-11003,
	-10702,
	-10394,
	-10080,
	-9760,
	-9434,
	-9102,
	-8765,
	-8423,
	-8076,
	-7723,
	-7366,
	-7005,
	-6639,
	-6270,
	-5897,
	-5520,
	-5139,
	-4756,
	-4370,
	-3981,
	-3590,
	-3196,
	-2801,
	-2404,
	-2006,
	-1606,
	-1205,
	-804,
	-402,
	0,
	402,
	804,
	1205,
	1606,
	2006,
	2404,
	2801,
	3196,
	3590,
	3981,
	4370,
	4756,
	5139,
	5520,
	5897,
	6270,
	6639,
	7005,
	7366,
	7723,
	8076,
	8423,
	8765,
	9102,
	9434,
	9760,
	10080,
	10394,
	10702,
	11003,
	11297,
	11585,
	11866,
	12140,
	12406,
	12665,
	12916,
	13160,
	13395,
	13623,
	13842,
	14053,
	14256,
	14449,
	14635,
	14811,
	14978,
	15137,
	15286,
	15426,
	15557,
	15679,
	15791,
	15893,
	15986,
	16069,
	16143,
	16207,
	16261,
	16305,
	16340,
	16364,
	16379,
	16384
};

ushort asin_table [] = {
	0,
	41,
	81,
	122,
	163,
	204,
	244,
	285,
	326,
	367,
	408,
	448,
	489,
	530,
	571,
	612,
	652,
	693,
	734,
	775,
	816,
	857,
	897,
	938,
	979,
	1020,
	1061,
	1102,
	1143,
	1184,
	1225,
	1266,
	1307,
	1348,
	1389,
	1431,
	1472,
	1513,
	1554,
	1595,
	1636,
	1678,
	1719,
	1760,
	1802,
	1843,
	1884,
	1926,
	1967,
	2009,
	2050,
	2092,
	2134,
	2175,
	2217,
	2259,
	2300,
	2342,
	2384,
	2426,
	2468,
	2510,
	2551,
	2593,
	2636,
	2678,
	2720,
	2762,
	2804,
	2847,
	2889,
	2931,
	2974,
	3016,
	3059,
	3101,
	3144,
	3187,
	3229,
	3272,
	3315,
	3358,
	3401,
	3444,
	3487,
	3530,
	3573,
	3617,
	3660,
	3704,
	3747,
	3791,
	3834,
	3878,
	3922,
	3965,
	4009,
	4053,
	4097,
	4142,
	4186,
	4230,
	4275,
	4319,
	4364,
	4408,
	4453,
	4498,
	4543,
	4588,
	4633,
	4678,
	4723,
	4768,
	4814,
	4859,
	4905,
	4951,
	4997,
	5043,
	5089,
	5135,
	5181,
	5228,
	5274,
	5321,
	5367,
	5414,
	5461,
	5508,
	5556,
	5603,
	5651,
	5698,
	5746,
	5794,
	5842,
	5890,
	5938,
	5987,
	6035,
	6084,
	6133,
	6182,
	6231,
	6281,
	6330,
	6380,
	6430,
	6480,
	6530,
	6580,
	6631,
	6681,
	6732,
	6783,
	6835,
	6886,
	6938,
	6990,
	7042,
	7094,
	7147,
	7199,
	7252,
	7306,
	7359,
	7413,
	7466,
	7521,
	7575,
	7630,
	7684,
	7740,
	7795,
	7851,
	7907,
	7963,
	8019,
	8076,
	8133,
	8191,
	8249,
	8307,
	8365,
	8424,
	8483,
	8543,
	8602,
	8663,
	8723,
	8784,
	8846,
	8907,
	8970,
	9032,
	9095,
	9159,
	9223,
	9288,
	9353,
	9418,
	9484,
	9551,
	9618,
	9686,
	9754,
	9823,
	9892,
	9963,
	10034,
	10105,
	10177,
	10251,
	10324,
	10399,
	10475,
	10551,
	10628,
	10706,
	10785,
	10866,
	10947,
	11029,
	11113,
	11198,
	11284,
	11371,
	11460,
	11550,
	11642,
	11736,
	11831,
	11929,
	12028,
	12130,
	12234,
	12340,
	12449,
	12561,
	12677,
	12796,
	12919,
	13046,
	13178,
	13315,
	13459,
	13610,
	13770,
	13939,
	14121,
	14319,
	14538,
	14786,
	15079,
	15462,
	16384,
	16384   // extra for when exactly 1
};

ushort acos_table [] = {
	16384,
	16343,
	16303,
	16262,
	16221,
	16180,
	16140,
	16099,
	16058,
	16017,
	15976,
	15936,
	15895,
	15854,
	15813,
	15772,
	15732,
	15691,
	15650,
	15609,
	15568,
	15527,
	15487,
	15446,
	15405,
	15364,
	15323,
	15282,
	15241,
	15200,
	15159,
	15118,
	15077,
	15036,
	14995,
	14953,
	14912,
	14871,
	14830,
	14789,
	14748,
	14706,
	14665,
	14624,
	14582,
	14541,
	14500,
	14458,
	14417,
	14375,
	14334,
	14292,
	14250,
	14209,
	14167,
	14125,
	14084,
	14042,
	14000,
	13958,
	13916,
	13874,
	13833,
	13791,
	13748,
	13706,
	13664,
	13622,
	13580,
	13537,
	13495,
	13453,
	13410,
	13368,
	13325,
	13283,
	13240,
	13197,
	13155,
	13112,
	13069,
	13026,
	12983,
	12940,
	12897,
	12854,
	12811,
	12767,
	12724,
	12680,
	12637,
	12593,
	12550,
	12506,
	12462,
	12419,
	12375,
	12331,
	12287,
	12242,
	12198,
	12154,
	12109,
	12065,
	12020,
	11976,
	11931,
	11886,
	11841,
	11796,
	11751,
	11706,
	11661,
	11616,
	11570,
	11525,
	11479,
	11433,
	11387,
	11341,
	11295,
	11249,
	11203,
	11156,
	11110,
	11063,
	11017,
	10970,
	10923,
	10876,
	10828,
	10781,
	10733,
	10686,
	10638,
	10590,
	10542,
	10494,
	10446,
	10397,
	10349,
	10300,
	10251,
	10202,
	10153,
	10103,
	10054,
	10004,
	9954,
	9904,
	9854,
	9804,
	9753,
	9703,
	9652,
	9601,
	9549,
	9498,
	9446,
	9394,
	9342,
	9290,
	9237,
	9185,
	9132,
	9078,
	9025,
	8971,
	8918,
	8863,
	8809,
	8754,
	8700,
	8644,
	8589,
	8533,
	8477,
	8421,
	8365,
	8308,
	8251,
	8193,
	8135,
	8077,
	8019,
	7960,
	7901,
	7841,
	7782,
	7721,
	7661,
	7600,
	7538,
	7477,
	7414,
	7352,
	7289,
	7225,
	7161,
	7096,
	7031,
	6966,
	6900,
	6833,
	6766,
	6698,
	6630,
	6561,
	6492,
	6421,
	6350,
	6279,
	6207,
	6133,
	6060,
	5985,
	5909,
	5833,
	5756,
	5678,
	5599,
	5518,
	5437,
	5355,
	5271,
	5186,
	5100,
	5013,
	4924,
	4834,
	4742,
	4648,
	4553,
	4455,
	4356,
	4254,
	4150,
	4044,
	3935,
	3823,
	3707,
	3588,
	3465,
	3338,
	3206,
	3069,
	2925,
	2774,
	2614,
	2445,
	2263,
	2065,
	1846,
	1598,
	1305,
	922,
	0,
	0   // extra for when exactly 1
};

fix isqrt_guess_table [] = {
	// 0,
	0x80000000,
	16777216,11863283,9686330,8388608,7502999,6849269,6341191,
	5931641,5592405,5305421,5058520,4843165,4653162,4483899,4331858,
	4194304,4069072,3954427,3848957,3751499,3661088,3576914,3498291,
	3424634,3355443,3290282,3228776,3170595,3115450,3063086,3013276,
	2965820,2920538,2877268,2835867,2796202,2758157,2721623,2686504,
	2652710,2620160,2588780,2558501,2529260,2500999,2473665,2447208,
	2421582,2396745,2372656,2349280,2326581,2304527,2283089,2262239,
	2241949,2222196,2202956,2184207,2165929,2148102,2130708,2113730,
	2097152,2080957,2065132,2049663,2034536,2019739,2005260,1991089,
	1977213,1963624,1950311,1937266,1924478,1911941,1899645,1887584,
	1875749,1864135,1852733,1841538,1830544,1819744,1809133,1798706,
	1788457,1778381,1768473,1758730,1749145,1739716,1730437,1721306,
	1712317,1703468,1694754,1686173,1677721,1669395,1661191,1653108,
	1645141,1637288,1629547,1621914,1614388,1606965,1599644,1592422,
	1585297,1578267,1571330,1564483,1557725,1551054,1544467,1537964,
	1531543,1525201,1518937,1512750,1506638,1500599,1494633,1488737,
	1482910,1477151,1471459,1465832,1460269,1454769,1449330,1443952,
	1438634,1433374,1428171,1423024,1417933,1412896,1407912,1402981,
	1398101,1393271,1388492,1383761,1379078,1374443,1369853,1365310,
	1360811,1356357,1351946,1347578,1343252,1338967,1334723,1330519,
	1326355,1322229,1318142,1314092,1310080,1306104,1302164,1298259,
	1294390,1290555,1286753,1282985,1279250,1275548,1271877,1268238,
	1264630,1261052,1257505,1253987,1250499,1247040,1243609,1240207,
	1236832,1233485,1230165,1226871,1223604,1220362,1217147,1213956,
	1210791,1207650,1204533,1201441,1198372,1195327,1192304,1189305,
	1186328,1183373,1180440,1177529,1174640,1171771,1168924,1166097,
	1163290,1160504,1157737,1154991,1152263,1149555,1146866,1144196,
	1141544,1138911,1136296,1133699,1131119,1128557,1126013,1123485,
	1120974,1118481,1116003,1113542,1111098,1108669,1106256,1103859,
	1101478,1099111,1096760,1094424,1092103,1089797,1087505,1085227,
	1082964,1080715,1078480,1076258,1074051,1071857,1069676,1067508,
	1065354,1063212,1061084,1058968,1056865,1054774,1052696,1050630,
	1048576,1046533,1044503,1042485,1040478,1038483,1036499,1034527,
	1032566,1030616,1028677,1026748,1024831,1022924,1021028,1019143,
	1017268,1015403,1013548,1011704,1009869,1008045,1006230,1004425,
	1002630,1000844,999068,997301,995544,993796,992057,990327,
	988606,986895,985192,983497,981812,980135,978467,976807,
	975155,973512,971878,970251,968633,967022,965420,963825,
	962239,960660,959089,957526,955970,954422,952881,951348,
	949822,948304,946793,945289,943792,942302,940819,939343,
	937874,936412,934957,933509,932067,930632,929204,927782,
	926366,924957,923555,922159,920769,919385,918008,916637,
	915272,913913,912560,911213,909872,908537,907207,905884,
	904566,903254,901948,900648,899353,898063,896779,895501,
	894228,892961,891698,890442,889190,887944,886703,885467,
	884236,883011,881790,880575,879365,878159,876959,875763,
	874572,873386,872205,871029,869858,868691,867529,866371,
	865218,864070,862926,861787,860653,859522,858397,857275,
	856158,855046,853937,852833,851734,850638,849547,848460,
	847377,846298,845223,844153,843086,842024,840965,839911,
	838860,837814,836771,835732,834697,833666,832639,831615,
	830595,829579,828567,827559,826554,825552,824555,823561,
	822570,821583,820600,819620,818644,817671,816702,815736,
	814773,813814,812858,811906,810957,810011,809069,808130,
	807194,806261,805332,804405,803482,802563,801646,800732,
	799822,798915,798010,797109,796211,795316,794424,793535,
	792648,791765,790885,790008,789133,788262,787393,786528,
	785665,784805,783947,783093,782241,781392,780546,779703,
	778862,778024,777189,776356,775527,774699,773875,773053,
	772233,771417,770603,769791,768982,768175,767371,766570,
	765771,764975,764181,763389,762600,761814,761029,760248,
	759468,758691,757917,757145,756375,755607,754842,754079,
	753319,752560,751805,751051,750299,749550,748803,748059,
	747316,746576,745838,745102,744368,743637,742907,742180,
	741455,740732,740011,739292,738575,737861,737148,736438,
	735729,735023,734318,733616,732916,732217,731521,730827,
	730134,729444,728755,728069,727384,726701,726021,725342,
	724665,723990,723317,722645,721976,721308,720643,719979,
	719317,718656,717998,717341,716687,716034,715382,714733,
	714085,713439,712795,712153,711512,710873,710236,709600,
	708966,708334,707704,707075,706448,705822,705198,704576,
	703956,703337,702720,702104,701490,700878,700267,699658,
	699050,698444,697840,697237,696635,696036,695437,694841,
	694246,693652,693060,692469,691880,691293,690707,690122,
	689539,688957,688377,687798,687221,686645,686071,685498,
	684926,684356,683788,683221,682655,682090,681527,680966,
	680405,679847,679289,678733,678178,677625,677073,676522,
	675973,675425,674878,674333,673789,673246,672705,672164,
	671626,671088,670552,670017,669483,668951,668420,667890,
	667361,666834,666308,665783,665259,664737,664216,663696,
	663177,662660,662143,661628,661114,660602,660090,659580,
	659071,658563,658056,657550,657046,656543,656041,655540,
	655040,654541,654043,653547,653052,652558,652065,651573,
	651082,650592,650103,649616,649129,648644,648160,647677,
	647195,646714,646234,645755,645277,644800,644325,643850,
	643376,642904,642432,641962,641492,641024,640557,640090,
	639625,639161,638697,638235,637774,637313,636854,636396,
	635938,635482,635026,634572,634119,633666,633215,632764,
	632315,631866,631418,630972,630526,630081,629637,629194,
	628752,628311,627871,627432,626993,626556,626120,625684,
	625249,624816,624383,623951,623520,623090,622660,622232,
	621804,621378,620952,620527,620103,619680,619258,618836,
	618416,617996,617577,617159,616742,616326,615910,615496,
	615082,614669,614257,613846,613435,613026,612617,612209,
	611802,611395,610990,610585,610181,609778,609375,608974,
	608573,608173,607774,607375,606978,606581,606185,605790,
	605395,605001,604608,604216,603825,603434,603044,602655,
	602266,601879,601492,601106,600720,600335,599952,599568,
	599186,598804,598423,598043,597663,597284,596906,596529,
	596152,595776,595401,595026,594652,594279,593907,593535,
	593164,592793,592424,592055,591686,591319,590952,590585,
	590220,589855,589491,589127,588764,588402,588041,587680,
	587320,586960,586601,586243,585885,585528,585172,584816,
	584462,584107,583754,583400,583048,582696,582345,581995,
	581645,581296,580947,580599,580252,579905,579559,579213,
	578868,578524,578181,577838,577495,577153,576812,576471,
	576131,575792,575453,575115,574777,574440,574104,573768,
	573433,573098,572764,572431,572098,571765,571434,571103,
	570772,570442,570113,569784,569455,569128,568800,568474,
	568148,567822,567497,567173,566849,566526,566203,565881,
	565559,565238,564918,564598,564278,563959,563641,563323,
	563006,562689,562373,562057,561742,561428,561114,560800,
	560487,560174,559862,559551,559240,558930,558620,558310,
	558001,557693,557385,557078,556771,556465,556159,555853,
	555549,555244,554940,554637,554334,554032,553730,553429,
	553128,552828,552528,552228,551929,551631,551333,551036,
	550739,550442,550146,549851,549555,549261,548967,548673,
	548380,548087,547795,547503,547212,546921,546631,546341,
	546051,545762,545474,545186,544898,544611,544324,544038,
	543752,543467,543182,542897,542613,542330,542047,541764,
	541482,541200,540919,540638,540357,540077,539798,539518,
	539240,538961,538683,538406,538129,537852,537576,537300,
	537025,536750,536476,536202,535928,535655,535382,535110,
	534838,534566,534295,534024,533754,533484,533214,532945,
	532677,532408,532140,531873,531606,531339,531073,530807,
	530542,530277,530012,529748,529484,529220,528957,528694,
	528432,528170,527909,527648,527387,527126,526866,526607,
	526348,526089,525830,525572,525315,525057,524800,524544
};
