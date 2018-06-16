// Copyright 2018 Ulf Adams
//
// The contents of this file may be used under the terms of the Apache License,
// Version 2.0.
//
//    (See accompanying file LICENSE-Apache or copy at
//     http://www.apache.org/licenses/LICENSE-2.0)
//
// Alternatively, the contents of this file may be used under the terms of
// the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE-Boost or copy at
//     https://www.boost.org/LICENSE_1_0.txt)
//
// Unless required by applicable law or agreed to in writing, this software
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.

#include <inttypes.h>
#include <math.h>

#include "third_party/gtest/gtest.h"

#if defined(__SIZEOF_INT128__) && !defined(_MSC_VER) && !defined(ONLY_64_BIT_OPS_RYU)

#include "ryu/mulshift128.h"

#elif defined(_MSC_VER) && !defined(ONLY_64_BIT_OPS_RYU) && defined(_M_X64) \
  && !defined(__clang__) // https://bugs.llvm.org/show_bug.cgi?id=37755

#define HAS_64_BIT_INTRINSICS
#include "ryu/mulshift128.h"

#endif

// #define DOUBLE_LOG10_2_DENOMINATOR 10000000ull
// #define DOUBLE_LOG10_2_NUMERATOR 3010299ull // DOUBLE_LOG10_2_DENOMINATOR * log_10(2)
// #define DOUBLE_LOG10_5_DENOMINATOR 10000000ull
// #define DOUBLE_LOG10_5_NUMERATOR 6989700ull // DOUBLE_LOG10_5_DENOMINATOR * log_10(5)
#define DOUBLE_LOG2_5_DENOMINATOR 10000000ull
#define DOUBLE_LOG2_5_NUMERATOR 23219280ull // DOUBLE_LOG2_5_DENOMINATOR * log_2(5)

static inline uint32_t double_pow5bits(int32_t e) {
  return e == 0
      ? 1
      // We need to round up in this case.
      : (uint32_t) ((e * DOUBLE_LOG2_5_NUMERATOR + DOUBLE_LOG2_5_DENOMINATOR - 1) / DOUBLE_LOG2_5_DENOMINATOR);
}

static const uint64_t DOUBLE_POW5_SPLIT[326][2] = {
 {                    0u,    72057594037927936u }, {                    0u,    90071992547409920u },
 {                    0u,   112589990684262400u }, {                    0u,   140737488355328000u },
 {                    0u,    87960930222080000u }, {                    0u,   109951162777600000u },
 {                    0u,   137438953472000000u }, {                    0u,    85899345920000000u },
 {                    0u,   107374182400000000u }, {                    0u,   134217728000000000u },
 {                    0u,    83886080000000000u }, {                    0u,   104857600000000000u },
 {                    0u,   131072000000000000u }, {                    0u,    81920000000000000u },
 {                    0u,   102400000000000000u }, {                    0u,   128000000000000000u },
 {                    0u,    80000000000000000u }, {                    0u,   100000000000000000u },
 {                    0u,   125000000000000000u }, {                    0u,    78125000000000000u },
 {                    0u,    97656250000000000u }, {                    0u,   122070312500000000u },
 {                    0u,    76293945312500000u }, {                    0u,    95367431640625000u },
 {                    0u,   119209289550781250u }, {  4611686018427387904u,    74505805969238281u },
 { 10376293541461622784u,    93132257461547851u }, {  8358680908399640576u,   116415321826934814u },
 {   612489549322387456u,    72759576141834259u }, { 14600669991935148032u,    90949470177292823u },
 { 13639151471491547136u,   113686837721616029u }, {  3213881284082270208u,   142108547152020037u },
 {  4314518811765112832u,    88817841970012523u }, {   781462496279003136u,   111022302462515654u },
 { 10200200157203529728u,   138777878078144567u }, { 13292654125893287936u,    86736173798840354u },
 {  7392445620511834112u,   108420217248550443u }, {  4628871007212404736u,   135525271560688054u },
 { 16728102434789916672u,    84703294725430033u }, {  7075069988205232128u,   105879118406787542u },
 { 18067209522111315968u,   132348898008484427u }, {  8986162942105878528u,    82718061255302767u },
 {  6621017659204960256u,   103397576569128459u }, {  3664586055578812416u,   129246970711410574u },
 { 16125424340018921472u,    80779356694631608u }, {  1710036351314100224u,   100974195868289511u },
 { 15972603494424788992u,   126217744835361888u }, {  9982877184015493120u,    78886090522101180u },
 { 12478596480019366400u,    98607613152626475u }, { 10986559581596820096u,   123259516440783094u },
 {  2254913720070624656u,    77037197775489434u }, { 12042014186943056628u,    96296497219361792u },
 { 15052517733678820785u,   120370621524202240u }, {  9407823583549262990u,    75231638452626400u },
 { 11759779479436578738u,    94039548065783000u }, { 14699724349295723422u,   117549435082228750u },
 {  4575641699882439235u,    73468396926392969u }, { 10331238143280436948u,    91835496157991211u },
 {  8302361660673158281u,   114794370197489014u }, {  1154580038986672043u,   143492962746861268u },
 {  9944984561221445835u,    89683101716788292u }, { 12431230701526807293u,   112103877145985365u },
 {  1703980321626345405u,   140129846432481707u }, { 17205888765512323542u,    87581154020301066u },
 { 12283988920035628619u,   109476442525376333u }, {  1519928094762372062u,   136845553156720417u },
 { 12479170105294952299u,    85528470722950260u }, { 15598962631618690374u,   106910588403687825u },
 {  5663645234241199255u,   133638235504609782u }, { 17374836326682913246u,    83523897190381113u },
 {  7883487353071477846u,   104404871487976392u }, {  9854359191339347308u,   130506089359970490u },
 { 10770660513014479971u,    81566305849981556u }, { 13463325641268099964u,   101957882312476945u },
 {  2994098996302961243u,   127447352890596182u }, { 15706369927971514489u,    79654595556622613u },
 {  5797904354682229399u,    99568244445778267u }, {  2635694424925398845u,   124460305557222834u },
 {  6258995034005762182u,    77787690973264271u }, {  3212057774079814824u,    97234613716580339u },
 { 17850130272881932242u,   121543267145725423u }, { 18073860448192289507u,    75964541966078389u },
 {  8757267504958198172u,    94955677457597987u }, {  6334898362770359811u,   118694596821997484u },
 { 13182683513586250689u,    74184123013748427u }, { 11866668373555425458u,    92730153767185534u },
 {  5609963430089506015u,   115912692208981918u }, { 17341285199088104971u,    72445432630613698u },
 { 12453234462005355406u,    90556790788267123u }, { 10954857059079306353u,   113195988485333904u },
 { 13693571323849132942u,   141494985606667380u }, { 17781854114260483896u,    88434366004167112u },
 {  3780573569116053255u,   110542957505208891u }, {   114030942967678664u,   138178696881511114u },
 {  4682955357782187069u,    86361685550944446u }, { 15077066234082509644u,   107952106938680557u },
 {  5011274737320973344u,   134940133673350697u }, { 14661261756894078100u,    84337583545844185u },
 {  4491519140835433913u,   105421979432305232u }, {  5614398926044292391u,   131777474290381540u },
 { 12732371365632458552u,    82360921431488462u }, {  6692092170185797382u,   102951151789360578u },
 { 17588487249587022536u,   128688939736700722u }, { 15604490549419276989u,    80430587335437951u },
 { 14893927168346708332u,   100538234169297439u }, { 14005722942005997511u,   125672792711621799u },
 { 15671105866394830300u,    78545495444763624u }, {  1142138259283986260u,    98181869305954531u },
 { 15262730879387146537u,   122727336632443163u }, {  7233363790403272633u,    76704585395276977u },
 { 13653390756431478696u,    95880731744096221u }, {  3231680390257184658u,   119850914680120277u },
 {  4325643253124434363u,    74906821675075173u }, { 10018740084832930858u,    93633527093843966u },
 {  3300053069186387764u,   117041908867304958u }, { 15897591223523656064u,    73151193042065598u },
 { 10648616992549794273u,    91438991302581998u }, {  4087399203832467033u,   114298739128227498u },
 { 14332621041645359599u,   142873423910284372u }, { 18181260187883125557u,    89295889943927732u },
 {  4279831161144355331u,   111619862429909666u }, { 14573160988285219972u,   139524828037387082u },
 { 13719911636105650386u,    87203017523366926u }, {  7926517508277287175u,   109003771904208658u },
 {   684774848491833161u,   136254714880260823u }, {  7345513307948477581u,    85159196800163014u },
 { 18405263671790372785u,   106448996000203767u }, { 18394893571310578077u,   133061245000254709u },
 { 13802651491282805250u,    83163278125159193u }, {  3418256308821342851u,   103954097656448992u },
 {  4272820386026678563u,   129942622070561240u }, {  2670512741266674102u,    81214138794100775u },
 { 17173198981865506339u,   101517673492625968u }, {  3019754653622331308u,   126897091865782461u },
 {  4193189667727651020u,    79310682416114038u }, { 14464859121514339583u,    99138353020142547u },
 { 13469387883465536574u,   123922941275178184u }, {  8418367427165960359u,    77451838296986365u },
 { 15134645302384838353u,    96814797871232956u }, {   471562554271496325u,   121018497339041196u },
 {  9518098633274461011u,    75636560836900747u }, {  7285937273165688360u,    94545701046125934u },
 { 18330793628311886258u,   118182126307657417u }, {  4539216990053847055u,    73863828942285886u },
 { 14897393274422084627u,    92329786177857357u }, {  4786683537745442072u,   115412232722321697u },
 { 14520892257159371055u,    72132645451451060u }, { 18151115321449213818u,    90165806814313825u },
 {  8853836096529353561u,   112707258517892282u }, {  1843923083806916143u,   140884073147365353u },
 { 12681666973447792349u,    88052545717103345u }, {  2017025661527576725u,   110065682146379182u },
 { 11744654113764246714u,   137582102682973977u }, {   422879793461572340u,    85988814176858736u },
 {   528599741826965425u,   107486017721073420u }, {   660749677283706782u,   134357522151341775u },
 {  7330497575943398595u,    83973451344588609u }, { 13774807988356636147u,   104966814180735761u },
 {  3383451930163631472u,   131208517725919702u }, { 15949715511634433382u,    82005323578699813u },
 {  6102086334260878016u,   102506654473374767u }, {  3015921899398709616u,   128133318091718459u },
 { 18025852251620051174u,    80083323807324036u }, {  4085571240815512351u,   100104154759155046u },
 { 14330336087874166247u,   125130193448943807u }, { 15873989082562435760u,    78206370905589879u },
 { 15230800334775656796u,    97757963631987349u }, {  5203442363187407284u,   122197454539984187u },
 {   946308467778435600u,    76373409087490117u }, {  5794571603150432404u,    95466761359362646u },
 { 16466586540792816313u,   119333451699203307u }, {  7985773578781816244u,    74583407312002067u },
 {  5370530955049882401u,    93229259140002584u }, {  6713163693812353001u,   116536573925003230u },
 { 18030785363914884337u,    72835358703127018u }, { 13315109668038829614u,    91044198378908773u },
 {  2808829029766373305u,   113805247973635967u }, { 17346094342490130344u,   142256559967044958u },
 {  6229622945628943561u,    88910349979403099u }, {  3175342663608791547u,   111137937474253874u },
 { 13192550366365765242u,   138922421842817342u }, {  3633657960551215372u,    86826513651760839u },
 { 18377130505971182927u,   108533142064701048u }, {  4524669058754427043u,   135666427580876311u },
 {  9745447189362598758u,    84791517238047694u }, {  2958436949848472639u,   105989396547559618u },
 { 12921418224165366607u,   132486745684449522u }, { 12687572408530742033u,    82804216052780951u },
 { 11247779492236039638u,   103505270065976189u }, {   224666310012885835u,   129381587582470237u },
 {  2446259452971747599u,    80863492239043898u }, { 12281196353069460307u,   101079365298804872u },
 { 15351495441336825384u,   126349206623506090u }, { 14206370669262903769u,    78968254139691306u },
 {  8534591299723853903u,    98710317674614133u }, { 15279925143082205283u,   123387897093267666u },
 { 14161639232853766206u,    77117435683292291u }, { 13090363022639819853u,    96396794604115364u },
 { 16362953778299774816u,   120495993255144205u }, { 12532689120651053212u,    75309995784465128u },
 { 15665861400813816515u,    94137494730581410u }, { 10358954714162494836u,   117671868413226763u },
 {  4168503687137865320u,    73544917758266727u }, {   598943590494943747u,    91931147197833409u },
 {  5360365506546067587u,   114913933997291761u }, { 11312142901609972388u,   143642417496614701u },
 {  9375932322719926695u,    89776510935384188u }, { 11719915403399908368u,   112220638669230235u },
 { 10038208235822497557u,   140275798336537794u }, { 10885566165816448877u,    87672373960336121u },
 { 18218643725697949000u,   109590467450420151u }, { 18161618638695048346u,   136988084313025189u },
 { 13656854658398099168u,    85617552695640743u }, { 12459382304570236056u,   107021940869550929u },
 {  1739169825430631358u,   133777426086938662u }, { 14922039196176308311u,    83610891304336663u },
 { 14040862976792997485u,   104513614130420829u }, {  3716020665709083144u,   130642017663026037u },
 {  4628355925281870917u,    81651261039391273u }, { 10397130925029726550u,   102064076299239091u },
 {  8384727637859770284u,   127580095374048864u }, {  5240454773662356427u,    79737559608780540u },
 {  6550568467077945534u,    99671949510975675u }, {  3576524565420044014u,   124589936888719594u },
 {  6847013871814915412u,    77868710555449746u }, { 17782139376623420074u,    97335888194312182u },
 { 13004302183924499284u,   121669860242890228u }, { 17351060901807587860u,    76043662651806392u },
 {  3242082053549933210u,    95054578314757991u }, { 17887660622219580224u,   118818222893447488u },
 { 11179787888887237640u,    74261389308404680u }, { 13974734861109047050u,    92826736635505850u },
 {  8245046539531533005u,   116033420794382313u }, { 16682369133275677888u,    72520887996488945u },
 {  7017903361312433648u,    90651109995611182u }, { 17995751238495317868u,   113313887494513977u },
 {  8659630992836983623u,   141642359368142472u }, {  5412269370523114764u,    88526474605089045u },
 { 11377022731581281359u,   110658093256361306u }, {  4997906377621825891u,   138322616570451633u },
 { 14652906532082110942u,    86451635356532270u }, {  9092761128247862869u,   108064544195665338u },
 {  2142579373455052779u,   135080680244581673u }, { 12868327154477877747u,    84425425152863545u },
 {  2250350887815183471u,   105531781441079432u }, {  2812938609768979339u,   131914726801349290u },
 {  6369772649532999991u,    82446704250843306u }, { 17185587848771025797u,   103058380313554132u },
 {  3035240737254230630u,   128822975391942666u }, {  6508711479211282048u,    80514359619964166u },
 { 17359261385868878368u,   100642949524955207u }, { 17087390713908710056u,   125803686906194009u },
 {  3762090168551861929u,    78627304316371256u }, {  4702612710689827411u,    98284130395464070u },
 { 15101637925217060072u,   122855162994330087u }, { 16356052730901744401u,    76784476871456304u },
 {  1998321839917628885u,    95980596089320381u }, {  7109588318324424010u,   119975745111650476u },
 { 13666864735807540814u,    74984840694781547u }, { 12471894901332038114u,    93731050868476934u },
 {  6366496589810271835u,   117163813585596168u }, {  3979060368631419896u,    73227383490997605u },
 {  9585511479216662775u,    91534229363747006u }, {  2758517312166052660u,   114417786704683758u },
 { 12671518677062341634u,   143022233380854697u }, {  1002170145522881665u,    89388895863034186u },
 { 10476084718758377889u,   111736119828792732u }, { 13095105898447972362u,   139670149785990915u },
 {  5878598177316288774u,    87293843616244322u }, { 16571619758500136775u,   109117304520305402u },
 { 11491152661270395161u,   136396630650381753u }, {   264441385652915120u,    85247894156488596u },
 {   330551732066143900u,   106559867695610745u }, {  5024875683510067779u,   133199834619513431u },
 { 10058076329834874218u,    83249896637195894u }, {  3349223375438816964u,   104062370796494868u },
 {  4186529219298521205u,   130077963495618585u }, { 14145795808130045513u,    81298727184761615u },
 { 13070558741735168987u,   101623408980952019u }, { 11726512408741573330u,   127029261226190024u },
 {  7329070255463483331u,    79393288266368765u }, { 13773023837756742068u,    99241610332960956u },
 { 17216279797195927585u,   124052012916201195u }, {  8454331864033760789u,    77532508072625747u },
 {  5956228811614813082u,    96915635090782184u }, {  7445286014518516353u,   121144543863477730u },
 {  9264989777501460624u,    75715339914673581u }, { 16192923240304213684u,    94644174893341976u },
 {  1794409976670715490u,   118305218616677471u }, {  8039035263060279037u,    73940761635423419u },
 {  5437108060397960892u,    92425952044279274u }, { 16019757112352226923u,   115532440055349092u },
 {   788976158365366019u,    72207775034593183u }, { 14821278253238871236u,    90259718793241478u },
 {  9303225779693813237u,   112824648491551848u }, { 11629032224617266546u,   141030810614439810u },
 { 11879831158813179495u,    88144256634024881u }, {  1014730893234310657u,   110180320792531102u },
 { 10491785653397664129u,   137725400990663877u }, {  8863209042587234033u,    86078375619164923u },
 {  6467325284806654637u,   107597969523956154u }, { 17307528642863094104u,   134497461904945192u },
 { 10817205401789433815u,    84060913690590745u }, { 18133192770664180173u,   105076142113238431u },
 { 18054804944902837312u,   131345177641548039u }, { 18201782118205355176u,    82090736025967524u },
 {  4305483574047142354u,   102613420032459406u }, { 14605226504413703751u,   128266775040574257u },
 {  2210737537617482988u,    80166734400358911u }, { 16598479977304017447u,   100208418000448638u },
 { 11524727934775246001u,   125260522500560798u }, {  2591268940807140847u,    78287826562850499u },
 { 17074144231291089770u,    97859783203563123u }, { 16730994270686474309u,   122324729004453904u },
 { 10456871419179046443u,    76452955627783690u }, {  3847717237119032246u,    95566194534729613u },
 {  9421332564826178211u,   119457743168412016u }, {  5888332853016361382u,    74661089480257510u },
 { 16583788103125227536u,    93326361850321887u }, { 16118049110479146516u,   116657952312902359u },
 { 16991309721690548428u,    72911220195563974u }, { 12015765115258409727u,    91139025244454968u },
 { 15019706394073012159u,   113923781555568710u }, {  9551260955736489391u,   142404726944460888u },
 {  5969538097335305869u,    89002954340288055u }, {  2850236603241744433u,   111253692925360069u },
};

#define POW5_TABLE_SIZE 26
static const uint64_t DOUBLE_POW5_TABLE[POW5_TABLE_SIZE] = {
1ull, 5ull, 25ull, 125ull, 625ull, 3125ull, 15625ull, 78125ull, 390625ull,
1953125ull, 9765625ull, 48828125ull, 244140625ull, 1220703125ull, 6103515625ull,
30517578125ull, 152587890625ull, 762939453125ull, 3814697265625ull,
19073486328125ull, 95367431640625ull, 476837158203125ull,
2384185791015625ull, 11920928955078125ull, 59604644775390625ull,
298023223876953125ull //, 1490116119384765625ull
};
static const uint64_t DOUBLE_POW5_SPLIT2[13][2] = {
 {                    0u,    72057594037927936u },
 { 10376293541461622784u,    93132257461547851u },
 { 15052517733678820785u,   120370621524202240u },
 {  6258995034005762182u,    77787690973264271u },
 { 14893927168346708332u,   100538234169297439u },
 {  4272820386026678563u,   129942622070561240u },
 {  7330497575943398595u,    83973451344588609u },
 { 18377130505971182927u,   108533142064701048u },
 { 10038208235822497557u,   140275798336537794u },
 {  7017903361312433648u,    90651109995611182u },
 {  6366496589810271835u,   117163813585596168u },
 {  9264989777501460624u,    75715339914673581u },
 { 17074144231291089770u,    97859783203563123u },
};
static const uint32_t POW5_OFFSETS[13] = {
0x00000000, 0x00000000, 0x00000000, 0x033c55be, 0x03db77d8, 0x0265ffb2,
0x00000800, 0x01a8ff56, 0x00000000, 0x0037a200, 0x00004000, 0x03fffffc,
0x00003ffe,
};

static inline void getMultiplicand(uint32_t index, uint64_t* result) {
  uint32_t base = index / POW5_TABLE_SIZE;
  uint32_t base2 = base * POW5_TABLE_SIZE;
  uint32_t offset = index - base2;
  const uint64_t* mul = DOUBLE_POW5_SPLIT2[base];
  uint64_t m = DOUBLE_POW5_TABLE[offset];
  uint64_t high1;
  uint64_t low1 = umul128(m, mul[1], &high1);
  uint64_t high0;
  uint64_t low0 = umul128(m, mul[0], &high0);
  uint64_t sum = high0 + low1;
  if (sum < high0) high1++; // overflow into high1
  // high1 | sum | low0
  uint32_t delta = double_pow5bits(base2 + offset) - double_pow5bits(base2);
  result[0] = shiftright128(low0, sum, delta) + ((POW5_OFFSETS[base] >> offset) & 1);
  result[1] = shiftright128(sum, high1, delta);
}

TEST(D2sTableTest, GetMultiplicand) {
  for (int i = 0; i < 326; i += POW5_TABLE_SIZE) {
    uint64_t m[2];
    getMultiplicand(i, m);
    ASSERT_EQ(m[0], DOUBLE_POW5_SPLIT[i][0]);
  }
}

TEST(D2sTableTest, ComputeOffsets) {
  uint32_t totalErrors = 0;
  uint32_t offsets[13] = {0};
  for (int i = 0; i < 326; i++) {
    uint64_t m[2];
    getMultiplicand(i, m);
    if (m[0] != DOUBLE_POW5_SPLIT[i][0]) {
      offsets[i / POW5_TABLE_SIZE] |= 1 << (i % POW5_TABLE_SIZE);
      totalErrors++;
    }
  }
  if (totalErrors != 0) {
    for (int i = 0; i < 13; i++) {
      printf("0x%08x,\n", offsets[i]);
    }
  }
  ASSERT_EQ(totalErrors, 0);
}
