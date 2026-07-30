/* NetHack 5.0	shknam.c	$NHDT-Date: 1781973067 2026/06/20 16:31:07 $  $NHDT-Branch: NetHack-5.0 $:$NHDT-Revision: 1.90 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2011. */
/* NetHack may be freely redistributed.  See license for details. */

/* shknam.c -- initialize a shop */

/**
 * @file shknam.c
 * @brief Turning a room into a shop: what it sells, and who keeps it.
 *
 * A shop is decided before it is stocked. The kind of shop determines what may
 * appear in it and in what proportion, and the shopkeeper's name is drawn from
 * a list belonging to that kind, which is why a lighting shop is kept by
 * someone with a different sort of name than a delicatessen.
 *
 * Stocking then fills the room with goods the shopkeeper owns -- not the
 * hero's, which is the distinction the whole shop economy rests on.
 *
 * @note A room can fail to become a shop: it may have too many doors, be the
 *       wrong shape, or lie somewhere shops are not allowed.
 * @warning Everything placed here belongs to the shopkeeper. An object that
 *          loses that association becomes free goods, so ownership must be set
 *          as the objects are created rather than afterwards.
 */

/**
 * @file shknam.c
 * @brief 방을 상점으로 만드는 일. 무엇을 팔고 누가 지키는가.
 *
 * 상점은 물건을 채우기 전에 먼저 종류가 정해진다. 종류가 그 안에 나타날 수 있는
 * 물건과 그 비율을 정하고, 주인의 이름도 그 종류에 속한 목록에서 뽑는다. 조명
 * 가게 주인과 식료품점 주인의 이름 느낌이 다른 이유다.
 *
 * 그다음 물건 채우기가 방을 주인 소유의 상품으로 채운다. 영웅의 것이 아니라는
 * 이 구분 위에 상점 경제 전체가 놓여 있다.
 *
 * @note 방이 상점이 되지 못할 수도 있다. 문이 너무 많거나, 모양이 맞지 않거나,
 *       상점이 허용되지 않는 곳에 있을 수 있다.
 * @warning 여기서 놓인 모든 것은 주인의 소유다. 그 연결을 잃은 물건은 임자 없는
 *          물건이 되므로, 소유권은 나중이 아니라 객체를 만들 때 설정해야 한다.
 */

#include "hack.h"

staticfn boolean stock_room_goodpos(struct mkroom *, int, int, int, int);
staticfn boolean veggy_item(struct obj * obj, int);
staticfn int shkveg(void);
staticfn void mkveggy_at(int, int);
staticfn void mkshobj_at(const struct shclass *, int, int, boolean);
staticfn void nameshk(struct monst *, const char *const *);
staticfn int good_shopdoor(struct mkroom *, coordxy *, coordxy *);
staticfn int shkinit(const struct shclass *, struct mkroom *);

/**
 * @def VEGETARIAN_CLASS
 * @brief A stand-in class for "any vegetarian food", used only in the shop tables.
 *
 * Vegetarian is not an object class -- it is a property that cuts across the food class -- but the shop tables can only hold classes and specific types. So it is given a number just past the real classes, and every place that reads a
 * table entry tests for it before treating the value as a class.
 *
 * @warning Deliberately outside the range of real classes so that it cannot collide with one, and undefined at the end of this file so nothing outside can use it. A caller elsewhere that received this value would index the object-class
 *          tables past their end.
 */
/**
 * @def VEGETARIAN_CLASS
 * @brief "채식 음식 아무것"을 위한 대역 부류. 상점 표에서만 쓰인다.
 *
 * 채식은 물건 부류가 아니다. 그것은 음식 부류를 가로지르는 성질이다. 그런데 상점 표는 부류와 특정 종류만 담을 수 있다. 그래서 진짜 부류 바로 다음의 번호가 주어지고, 표 항목을 읽는 모든 곳이 그 값을 부류로 다루기 전에 그것을 검사한다.
 *
 * @warning 진짜 부류와 부딪칠 수 없도록 의도적으로 그 범위 밖이며, 밖의 무엇도 그것을 쓸 수 없도록 이 파일 끝에서 정의가 해제된다. 이 값을 받은 다른 곳의 호출자는 물건 부류 표를 그 끝을 넘어 색인할 것이다.
 */
#define VEGETARIAN_CLASS (MAXOCLASSES + 1)

/**
 * @name Shopkeeper name lists
 *
 * One list per kind of shop, so a shop's keeper has a name that suits what they sell.
 *
 * Each name carries a leading punctuation character encoding two things at once: the shopkeeper's gender, and whether the name is personal or general. That is why the lists cannot simply be read as text -- the prefix is stored with the
 * name and stripped only when the name is shown.
 *
 * The gender part matters because it decides which pronouns the shopkeeper is described with. The personal-or-general part decides the honorific: a general name becomes "Mr. Nikolai", while a personal name stands alone, since a person
 * known by their given name is not addressed with a title.
 *
 * @note Not every entry carries a prefix. An unprefixed name is read as a male general name, which is why the tool shop's list -- mostly unprefixed -- is not an error. Lists can mix the two freely, as the health food store's does.
 * @note Every list ends with a null entry rather than carrying a count, and the code finds each list's length by walking to it. That is what lets a list be extended by adding a line and nothing else.
 * @warning The prefix characters must stay outside the set of letters, because that is how the code recognises a prefix -- it tests whether the first character is a letter. A prefix that were a letter would be taken as part of the name.
 * @{
 */

/*
 *  Name prefix codes:
 *      dash          -  female, personal name
 *      underscore    _  female, general name
 *      plus          +  male, personal name
 *      vertical bar  |  male, general name (implied for most of shktools)
 *      equals        =  gender not specified, personal name
 *
 *  Personal names do not receive the honorific prefix "Mr." or "Ms.".
 */

/**
 * @name 상점 주인 이름 목록
 *
 * 상점 종류마다 한 목록이어서, 상점 주인은 자기가 파는 것에 어울리는 이름을 갖는다.
 *
 * 각 이름은 두 가지를 한 번에 부호화하는 앞선 기호 문자를 지닌다. 주인의 성별, 그리고 그 이름이 개인 이름인지 일반 이름인지. 그것이 목록이 그저 글로 읽힐 수 없는 이유다. 접두 문자는 이름과 함께 저장되고 이름이 보일 때에만 떼어진다.
 *
 * 성별 부분이 중요한 것은 그것이 주인이 어떤 대명사로 서술되는지를 정하기 때문이다. 개인이냐 일반이냐 하는 부분은 존칭을 정한다. 일반 이름은 "니콜라이 씨"가 되고, 개인 이름은 홀로 선다. 자기 이름으로 알려진 사람은 직함으로 불리지 않기 때문이다.
 *
 * @note 모든 항목이 접두 문자를 지니는 것은 아니다. 접두 문자 없는 이름은 남성 일반 이름으로 읽히며, 그것이 대부분 접두 문자가 없는 도구 상점 목록이 오류가 아닌 이유다. 목록은 건강식품점 목록이 그러하듯 그 둘을 자유로이 섞을 수 있다.
 * @note 모든 목록이 개수를 지니는 대신 널 항목으로 끝나며, 코드가 각 목록의 길이를 그것까지 걸어가서 찾는다. 그것이 한 줄을 더하는 것 말고 아무것도 하지 않고 목록을 늘릴 수 있게 하는 것이다.
 * @warning 접두 문자들은 글자 집합 밖에 머물러야 한다. 코드가 접두 문자를 알아보는 방법이 그것이기 때문이다. 그것은 첫 문자가 글자인지를 검사한다. 글자인 접두 문자는 이름의 일부로 여겨질 것이다.
 * @{
 */

static const char *const shkliquors[] = {
    /* Ukraine */
    "Njezjin", "Tsjernigof", "Ossipewsk", "Gorlowka",
    /* Belarus */
    "Gomel",
    /* N. Russia */
    "Konosja", "Weliki Oestjoeg", "Syktywkar", "Sablja", "Narodnaja", "Kyzyl",
    /* Silezie */
    "Walbrzych", "Swidnica", "Klodzko", "Raciborz", "Gliwice", "Brzeg",
    "Krnov", "Hradec Kralove",
    /* Schweiz */
    "Leuk", "Brig", "Brienz", "Thun", "Sarnen", "Burglen", "Elm", "Flims",
    "Vals", "Schuls", "Zum Loch", 0
};

static const char *const shkbooks[] = {
    /* Eire */
    "Skibbereen",  "Kanturk",   "Rath Luirc",     "Ennistymon",
    "Lahinch",     "Kinnegad",  "Lugnaquillia",   "Enniscorthy",
    "Gweebarra",   "Kittamagh", "Nenagh",         "Sneem",
    "Ballingeary", "Kilgarvan", "Cahersiveen",    "Glenbeigh",
    "Kilmihil",    "Kiltamagh", "Droichead Atha", "Inniscrone",
    "Clonegal",    "Lisnaskea", "Culdaff",        "Dunfanaghy",
    "Inishbofin",  "Kesh",      0
};

static const char *const shkarmors[] = {
    /* Turquie */
    "Demirci",    "Kalecik",    "Boyabai",    "Yildizeli", "Gaziantep",
    "Siirt",      "Akhalataki", "Tirebolu",   "Aksaray",   "Ermenak",
    "Iskenderun", "Kadirli",    "Siverek",    "Pervari",   "Malasgirt",
    "Bayburt",    "Ayancik",    "Zonguldak",  "Balya",     "Tefenni",
    "Artvin",     "Kars",       "Makharadze", "Malazgirt", "Midyat",
    "Birecik",    "Kirikkale",  "Alaca",      "Polatli",   "Nallihan",
    0
};

static const char *const shkwands[] = {
    /* Wales */
    "Yr Wyddgrug", "Trallwng", "Mallwyd", "Pontarfynach", "Rhaeader",
    "Llandrindod", "Llanfair-ym-muallt", "Y-Fenni", "Maesteg", "Rhydaman",
    "Beddgelert", "Curig", "Llanrwst", "Llanerchymedd", "Caergybi",
    /* Scotland */
    "Nairn", "Turriff", "Inverurie", "Braemar", "Lochnagar", "Kerloch",
    "Beinn a Ghlo", "Drumnadrochit", "Morven", "Uist", "Storr",
    "Sgurr na Ciche", "Cannich", "Gairloch", "Kyleakin", "Dunvegan", 0
};

static const char *const shkrings[] = {
    /* Hollandse familienamen */
    "Feyfer",     "Flugi",         "Gheel",      "Havic",   "Haynin",
    "Hoboken",    "Imbyze",        "Juyn",       "Kinsky",  "Massis",
    "Matray",     "Moy",           "Olycan",     "Sadelin", "Svaving",
    "Tapper",     "Terwen",        "Wirix",      "Ypey",
    /* Skandinaviske navne */
    "Rastegaisa", "Varjag Njarga", "Kautekeino", "Abisko",  "Enontekis",
    "Rovaniemi",  "Avasaksa",      "Haparanda",  "Lulea",   "Gellivare",
    "Oeloe",      "Kajaani",       "Fauske",     0
};

static const char *const shkfoods[] = {
    /* Indonesia */
    "Djasinga",    "Tjibarusa",   "Tjiwidej",      "Pengalengan",
    "Bandjar",     "Parbalingga", "Bojolali",      "Sarangan",
    "Ngebel",      "Djombang",    "Ardjawinangun", "Berbek",
    "Papar",       "Baliga",      "Tjisolok",      "Siboga",
    "Banjoewangi", "Trenggalek",  "Karangkobar",   "Njalindoeng",
    "Pasawahan",   "Pameunpeuk",  "Patjitan",      "Kediri",
    "Pemboeang",   "Tringanoe",   "Makin",         "Tipor",
    "Semai",       "Berhala",     "Tegal",         "Samoe",
    0
};

static const char *const shkweapons[] = {
    /* Perigord */
    "Voulgezac",   "Rouffiac",   "Lerignac",   "Touverac",  "Guizengeard",
    "Melac",       "Neuvicq",    "Vanzac",     "Picq",      "Urignac",
    "Corignac",    "Fleac",      "Lonzac",     "Vergt",     "Queyssac",
    "Liorac",      "Echourgnac", "Cazelon",    "Eypau",     "Carignan",
    "Monbazillac", "Jonzac",     "Pons",       "Jumilhac",  "Fenouilledes",
    "Laguiolet",   "Saujon",     "Eymoutiers", "Eygurande", "Eauze",
    "Labouheyre",  0
};

static const char *const shktools[] = {
    /* Spmi */
    "Ymla", "Eed-morra", "Elan Lapinski", "Cubask", "Nieb", "Bnowr Falr",
    "Sperc", "Noskcirdneh", "Yawolloh", "Hyeghu", "Niskal", "Trahnil",
    "Htargcm", "Enrobwem", "Kachzi Rellim", "Regien", "Donmyar", "Yelpur",
    "Nosnehpets", "Stewe", "Renrut", "Senna Hut", "-Zlaw", "Nosalnef",
    "Rewuorb", "Rellenk", "Yad", "Cire Htims", "Y-crad", "Nenilukah",
    "Corsh", "Aned", "Dark Eery", "Niknar", "Lapu", "Lechaim",
    "Rebrol-nek", "AlliWar Wickson", "Oguhmk", "Telloc Cyaj",
#ifdef OVERLAY
    "Erreip", "Nehpets", "Mron", "Snivek", "Kahztiy",
#endif
#ifdef WIN32
    "Lexa", "Niod",
#endif
#ifdef MAC68K
    "Nhoj-lee", "Evad\'kh", "Ettaw-noj", "Tsew-mot", "Ydna-s", "Yao-hang",
    "Tonbar", "Kivenhoug", "Llardom",
#endif
#ifdef AMIGA
    "Falo", "Nosid-da\'r", "Ekim-p", "Noslo", "Yl-rednow", "Mured-oog",
    "Ivrajimsal",
#endif
#ifdef TOS
    "Nivram",
#endif
#ifdef OS2
    "Nedraawi-nav",
#endif
#ifdef VMS
    "Lez-tneg", "Ytnu-haled",
#endif
    0
};

static const char *const shklight[] = {
    /* Romania */
    "Zarnesti", "Slanic", "Nehoiasu", "Ludus", "Sighisoara", "Nisipitu",
    "Razboieni", "Bicaz", "Dorohoi", "Vaslui", "Fetesti", "Tirgu Neamt",
    "Babadag", "Zimnicea", "Zlatna", "Jiu", "Eforie", "Mamaia",
    /* Bulgaria */
    "Silistra", "Tulovo", "Panagyuritshte", "Smolyan", "Kirklareli", "Pernik",
    "Lom", "Haskovo", "Dobrinishte", "Varvara", "Oryahovo", "Troyan",
    "Lovech", "Sliven", 0
};

static const char *const shkgeneral[] = {
    /* Suriname */
    "Hebiwerie",    "Possogroenoe", "Asidonhopo",   "Manlobbi",
    "Adjama",       "Pakka Pakka",  "Kabalebo",     "Wonotobo",
    "Akalapi",      "Sipaliwini",
    /* Greenland */
    "Annootok",     "Upernavik",    "Angmagssalik",
    /* N. Canada */
    "Aklavik",      "Inuvik",       "Tuktoyaktuk",  "Chicoutimi",
    "Ouiatchouane", "Chibougamau",  "Matagami",     "Kipawa",
    "Kinojevis",    "Abitibi",      "Maganasipi",
    /* Iceland */
    "Akureyri",     "Kopasker",     "Budereyri",    "Akranes",
    "Bordeyri",     "Holmavik",     0
};

static const char *const shkhealthfoods[] = {
    /* Tibet */
    "Ga'er",    "Zhangmu",   "Rikaze",   "Jiangji",     "Changdu",
    "Linzhi",   "Shigatse",  "Gyantse",  "Ganden",      "Tsurphu",
    "Lhasa",    "Tsedong",   "Drepung",
    /* Hippie names */
    "=Azura",   "=Blaze",    "=Breanna", "=Breezy",     "=Dharma",
    "=Feather", "=Jasmine",  "=Luna",    "=Melody",     "=Moonjava",
    "=Petal",   "=Rhiannon", "=Starla",  "=Tranquilla", "=Windsong",
    "=Zennia",  "=Zoe",      "=Zora",    0
};

/** @} */

/*
 * To add new shop types, all that is necessary is to edit the shtypes[]
 * array.  See mkroom.h for the structure definition.  Typically, you'll
 * have to lower some or all of the probability fields in old entries to
 * free up some percentage for the new type.
 *
 * The placement type field is not yet used but might be someday.
 *
 * The iprobs array in each entry defines the probabilities for various kinds
 * of objects to be present in the given shop type.  You can associate with
 * each percentage either a generic object type (represented by one of the
 * *_CLASS enum value) or a specific object enum value.
 * In the latter case, prepend it with a unary minus so the code can know
 * (by testing the sign) whether to use mkobj() or mksobj().
 * shtypes[] is externally referenced from mkroom.c, mon.c and shk.c.
 *
 * The second, usually shorter, store type name is used in automatically
 * generated annotations for #overview.  If Null, the first name gets used.
 */
/**
 * @var shtypes
 * @brief Every kind of shop: how likely it is, what it stocks, and who keeps it.
 *
 * The whole definition of what a shop is. Each entry gives the shop a name, a symbol standing for what it mainly deals in, how often it is generated, and a set of proportions saying what its stock is drawn from -- plus the name list its
 * keeper is named from.
 *
 * The entries with no probability are the unique shops, which are placed deliberately rather than at random. Code elsewhere finds them by looking for that zero, so the ordinary shops must all come first.
 *
 * @note Read from elsewhere as well -- room-making, monster code and the shop economy all consult it, as the existing comment records -- so this is a shared definition rather than a private table.
 * @note Each entry may carry a second, shorter name used in the automatically generated level annotations. Where there is none, the long name is used, as the existing comment records. The short name exists because "used armor
 *       dealership" is too long to sit in a level overview.
 * @note The placement-style field is unused, as the existing comment concedes; every shop is placed the one way.
 * @warning Both sets of probabilities must total a hundred: the shop probabilities across entries, and each entry's item proportions within itself. Neither is checked -- the check that would have done it is disabled -- and a total that
 *          is short lets the selection walk past the end of the table. The comment above tells an editor adding a shop to lower the others, and that instruction is the only thing enforcing it.
 */
/**
 * @var shtypes
 * @brief 모든 종류의 상점. 그것이 얼마나 있을 법한지, 무엇을 갖추는지, 누가 지키는지.
 *
 * 상점이 무엇인지에 대한 정의 전체다. 각 항목은 상점에 이름, 그것이 주로 다루는 것을 대신하는 기호, 얼마나 자주 생성되는지, 그리고 그 재고가 무엇에서 뽑히는지 말하는 비율 묶음을 준다. 더해서 그 주인이 이름 지어지는 이름 목록도.
 *
 * 확률이 없는 항목들은 유일한 상점이며, 무작위가 아니라 의도적으로 놓인다. 다른 곳의 코드가 그 영을 찾아 그것들을 발견하므로, 보통 상점들이 모두 먼저 와야 한다.
 *
 * @note 다른 곳에서도 읽힌다. 기존 주석이 기록하듯 방 만들기, 몬스터 코드, 상점 경제가 모두 그것을 참조하므로, 이것은 사적인 표가 아니라 공유된 정의다.
 * @note 각 항목은 자동으로 생성되는 층 주석에 쓰이는 두 번째의 더 짧은 이름을 지닐 수 있다. 기존 주석이 기록하듯 없는 곳에서는 긴 이름이 쓰인다. 짧은 이름이 존재하는 것은 "중고 갑옷 대리점"이 층 개관에 들어앉기에 너무 길기 때문이다.
 * @note 기존 주석이 인정하듯 배치 방식 항목은 쓰이지 않는다. 모든 상점이 그 한 방식으로 놓인다.
 * @warning 두 확률 묶음 모두 백을 합해야 한다. 항목들에 걸친 상점 확률, 그리고 각 항목 안의 물건 비율. 어느 것도 검사되지 않으며, 그것을 했을 검사는 비활성화되어 있다. 그리고 모자란 합계는 선택이 표의 끝을 넘어 걸어가게 한다. 위의 주석이 상점을 더하는 편집자에게 다른 것들을 낮추라고 말하며, 그 지시가 그것을 강제하는 유일한 것이다.
 */
const struct shclass shtypes[] = {
    { "general store", NULL,
      RANDOM_CLASS,
      42,
      D_SHOP,
      { { 100, RANDOM_CLASS },
        { 0, 0 },
        { 0, 0 },
        { 0, 0 },
        { 0, 0 },
        { 0, 0 } },
      shkgeneral },
    { "used armor dealership", "armor shop",
      ARMOR_CLASS,
      14,
      D_SHOP,
      { { 90, ARMOR_CLASS },
        { 10, WEAPON_CLASS },
        { 0, 0 },
        { 0, 0 },
        { 0, 0 },
        { 0, 0 } },
      shkarmors },
    { "second-hand bookstore", "scroll shop",
      SCROLL_CLASS,
      10,
      D_SHOP,
      { { 90, SCROLL_CLASS },
        { 10, SPBOOK_CLASS },
        { 0, 0 },
        { 0, 0 },
        { 0, 0 },
        { 0, 0 } },
      shkbooks },
    { "liquor emporium", "potion shop",
      POTION_CLASS,
      10,
      D_SHOP,
      { { 100, POTION_CLASS },
        { 0, 0 },
        { 0, 0 },
        { 0, 0 },
        { 0, 0 },
        { 0, 0 } },
      shkliquors },
    { "antique weapons outlet", "weapon shop",
      WEAPON_CLASS,
      5,
      D_SHOP,
      { { 90, WEAPON_CLASS },
        { 10, ARMOR_CLASS },
        { 0, 0 },
        { 0, 0 },
        { 0, 0 },
        { 0, 0 } },
      shkweapons },
    { "delicatessen", "food shop",
      FOOD_CLASS,
      5,
      D_SHOP,
      { { 83, FOOD_CLASS },
        { 5, -POT_FRUIT_JUICE },
        { 4, -POT_BOOZE },
        { 5, -POT_WATER },
        { 3, -ICE_BOX },
        { 0, 0 } },
      shkfoods },
    { "jewelers", "ring shop",
      RING_CLASS,
      3,
      D_SHOP,
      { { 85, RING_CLASS },
        { 10, GEM_CLASS },
        { 5, AMULET_CLASS },
        { 0, 0 },
        { 0, 0 },
        { 0, 0 } },
      shkrings },
    { "quality apparel and accessories", "wand shop",
      WAND_CLASS,
      3,
      D_SHOP,
      { { 90, WAND_CLASS },
        { 5, -LEATHER_GLOVES },
        { 5, -ELVEN_CLOAK },
        { 0, 0 } },
      shkwands },
    { "hardware store", "tool shop",
      TOOL_CLASS,
      3,
      D_SHOP,
      { { 100, TOOL_CLASS },
        { 0, 0 },
        { 0, 0 },
        { 0, 0 },
        { 0, 0 },
        { 0, 0 } },
      shktools },
    { "rare books", "bookstore",
      SPBOOK_CLASS,
      3,
      D_SHOP,
      { { 90, SPBOOK_CLASS },
        { 10, SCROLL_CLASS },
        { 0, 0 },
        { 0, 0 },
        { 0, 0 },
        { 0, 0 } },
      shkbooks },
    { "health food store", "vegetarian food shop",
      FOOD_CLASS,
      2,
      D_SHOP,
      { { 70, VEGETARIAN_CLASS },
        { 20, -POT_FRUIT_JUICE },
        { 4, -POT_HEALING },
        { 3, -POT_FULL_HEALING },
        { 2, -SCR_FOOD_DETECTION },
        { 1, -LUMP_OF_ROYAL_JELLY } },
      shkhealthfoods },
    /* Shops below this point are "unique".  That is they must all have a
     * probability of zero.  They are only created via the special level
     * loader.
     */
    { "lighting store", "lighting shop",
      TOOL_CLASS,
      0,
      D_SHOP,
      { { 30, -WAX_CANDLE },
        { 44, -TALLOW_CANDLE },
        { 5, -BRASS_LANTERN },
        { 9, -OIL_LAMP },
        { 3, -MAGIC_LAMP },
        { 5, -POT_OIL },
        { 2, -WAN_LIGHT },
        { 1, -SCR_LIGHT },
        { 1, -SPE_LIGHT } },
      shklight },
    /* sentinel */
    { (char *) 0, NULL,
      0,
      0,
      0,
      { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
      0 }
};

/**
 * @brief Check that the shop and item probabilities each total a hundred.
 *
 * Disabled, but kept because the reason for it is still true. The tables are chosen from by drawing a number in a range and walking until the total is exceeded, so a set of probabilities that does not total a hundred does not merely
 * skew the outcome -- as the existing comment warns, it can loop forever or index past the end of the table and read whatever is there.
 *
 * @note The failure it guards against is one a person makes while editing the tables: adding a shop type without lowering the others to make room. That is exactly what the comment above the tables tells an editor to do, which is why
 *       the check is worth keeping even unused.
 */
/**
 * @brief 상점 확률과 물건 확률이 각각 백을 합해 이루는지 검사한다.
 *
 * 비활성화되어 있지만, 그것을 위한 이유가 여전히 참이기 때문에 남겨져 있다. 표는 어떤 범위에서 수를 뽑고 합계를 넘길 때까지 걸어가며 고르므로, 백을 합하지 않는 확률 묶음은 결과를 기울게 하는 데 그치지 않는다. 기존 주석이 경고하듯, 그것은 영원히 돌 수도 있고 표의 끝을 넘어 색인해 그곳에 있는 무엇이든 읽을 수도 있다.
 *
 * @note 이것이 막는 실패는 사람이 표를 편집하면서 저지르는 것이다. 자리를 만들기 위해 다른 것들을 낮추지 않고 상점 종류를 더하는 일. 그것이 바로 표 위의 주석이 편집자에게 하라고 말하는 일이며, 그래서 이 검사가 쓰이지 않더라도 남겨 둘 만하다.
 */
#if 0
/* validate shop probabilities; otherwise incorrect local changes could
   end up provoking infinite loops or wild subscripts fetching garbage */
void
init_shop_selection()
{
    int i, j, item_prob, shop_prob;

    for (shop_prob = 0, i = 0; i < SIZE(shtypes); i++) {
        shop_prob += shtypes[i].prob;
        for (item_prob = 0, j = 0; j < SIZE(shtypes[0].iprobs); j++)
            item_prob += shtypes[i].iprobs[j].iprob;
        if (item_prob != 100)
            panic("item probabilities total to %d for %s shops!",
                  item_prob, shtypes[i].name);
    }
    if (shop_prob != 100)
        panic("shop probabilities total to %d!", shop_prob);
}
#endif /*0*/

/**
 * @brief Would this count as vegetarian food?
 *
 * Answers for either a particular object or a bare type, and the two answers are not equally good. An actual object knows what species its corpse is and what is inside its tin; a type can only say what such objects usually are. So
 * for a type the doubtful cases are assumed vegetarian, as the existing comment records -- which is right for stocking a shop, where the object does not exist yet and the assumption is corrected when it is made.
 *
 * @param obj the object to judge, or null to judge a type
 * @param otyp the type to judge; consulted only when @p obj is null
 * @return whether it is vegetarian
 * @warning A tin with nothing identified in it is judged by its enchantment field, which for tins holds the contents rather than an enchantment. Reading that field as an enchantment gives a plausible small number and no sign of being
 *          wrong.
 * @note Anything not food is not vegetarian, which reads oddly but is what the callers want: they are choosing stock for a health food store, and a sword is not something it sells.
 */
/**
 * @brief 이것이 채식 음식으로 셈될 것인가?
 *
 * 특정한 물건에 대해서든 맨 종류에 대해서든 답하지만, 그 두 답은 똑같이 좋지 않다. 실제 물건은 자기 시체가 어떤 종인지, 자기 통조림 안에 무엇이 있는지 안다. 종류는 그런 물건이 보통 무엇인지만 말할 수 있다. 그래서 종류에 대해서는 의심스러운 경우가 채식으로 가정된다. 기존 주석이 기록하는 대로다. 그것은 상점에 물건을 채우는 데는 옳다. 거기서 물건은 아직 존재하지 않으며, 만들어질 때 그 가정이 바로잡힌다.
 *
 * @param obj 판단할 물건, 또는 종류를 판단하려면 널
 * @param otyp 판단할 종류. @p obj 가 널일 때만 참조된다
 * @return 그것이 채식인지
 * @warning 안에 무엇이 있는지 밝혀지지 않은 통조림은 자기 강화 값 항목으로 판단되는데, 통조림에서 그 항목은 강화가 아니라 내용물을 담는다. 그 항목을 강화로 읽으면 그럴듯한 작은 수가 나오고 틀렸다는 표시는 없다.
 * @note 음식이 아닌 것은 채식이 아니며, 그것은 이상하게 읽히지만 호출자들이 원하는 것이다. 그들은 건강식품점의 재고를 고르고 있고, 검은 그것이 파는 것이 아니다.
 */
staticfn boolean
veggy_item(struct obj* obj, int otyp /* used iff obj is null */)
{
    int corpsenm;
    char oclass;

    if (obj) {
        /* actual object; will check tin content and corpse species */
        otyp = (int) obj->otyp;
        oclass = obj->oclass;
        corpsenm = obj->corpsenm;
    } else {
        /* just a type; caller will have to handle tins and corpses */
        oclass = objects[otyp].oc_class;
        corpsenm = PM_LICHEN; /* veggy standin */
    }

    if (oclass == FOOD_CLASS) {
        if (objects[otyp].oc_material == VEGGY || otyp == EGG)
            return TRUE;
        if (otyp == TIN && corpsenm == NON_PM) /* implies obj is non-null */
            return (boolean) (obj->spe == 1); /* 0 = empty, 1 = spinach */
        if (otyp == TIN || otyp == CORPSE)
            return (boolean) (ismnum(corpsenm) && vegetarian(&mons[corpsenm]));
    }
    return FALSE;
}

/**
 * @brief Choose a vegetarian food type, weighted the way food normally appears.
 *
 * Cannot use the ordinary object-making machinery, because that would choose from all food and most food is not vegetarian. So it collects the acceptable types first, sums only their weights, and draws within that sum -- which keeps
 * the relative frequencies food normally has while excluding everything a health food store would not sell.
 *
 * @return an object type
 * @note Relies on food types being contiguous, stopping at the first type of another class. That holds because the object table is grouped by class, and if it ever stopped holding this would silently stock a health food store with
 *       whatever follows food.
 * @note Ends the game rather than returning if nothing acceptable exists or the walk lands somewhere impossible. Both would mean the object table is not shaped as assumed, and a wrong answer here would be an object placed with no
 *       indication of the cause.
 */
/**
 * @brief 음식이 보통 나타나는 방식대로 가중치를 두어 채식 음식 종류를 고른다.
 *
 * 보통의 물건 만들기 기계를 쓸 수 없다. 그것은 모든 음식에서 고를 것이고, 대부분의 음식은 채식이 아니기 때문이다. 그래서 받아들일 만한 종류를 먼저 모으고, 그것들의 가중치만 합하고, 그 합 안에서 뽑는다. 그것이 건강식품점이 팔지 않을 모든 것을 배제하면서 음식이 보통 갖는 상대적 빈도를 유지한다.
 *
 * @return 물건 종류
 * @note 음식 종류가 이어져 있다는 것에 의존하며, 다른 부류의 첫 종류에서 멈춘다. 물건 표가 부류별로 묶여 있으므로 그것이 성립한다. 그것이 성립하기를 그친다면 이것은 조용히 건강식품점에 음식 다음에 오는 무엇이든 채울 것이다.
 * @note 받아들일 만한 것이 없거나 걸어가기가 있을 수 없는 곳에 닿으면 값을 돌려주는 대신 게임을 끝낸다. 둘 다 물건 표가 가정된 모양이 아니라는 뜻이며, 여기서의 틀린 답은 원인의 표시 없이 놓인 물건이 될 것이다.
 */
staticfn int
shkveg(void)
{
    int i, j, maxprob, prob;
    char oclass = FOOD_CLASS;
    int ok[NUM_OBJECTS];

    (void) memset((genericptr_t) ok, 0, sizeof ok); /* lint suppression */
    j = maxprob = 0;
    for (i = svb.bases[(int) oclass]; i < NUM_OBJECTS; ++i) {
        if (objects[i].oc_class != oclass)
            break;

        if (veggy_item((struct obj *) 0, i)) {
            ok[j++] = i;
            maxprob += objects[i].oc_prob;
        }
    }
    if (maxprob < 1)
        panic("shkveg no veggy objects");
    prob = rnd(maxprob);

    j = 0;
    i = ok[0];
    while ((prob -= objects[i].oc_prob) > 0) {
        j++;
        i = ok[j];
    }

    if (objects[i].oc_class != oclass || !OBJ_NAME(objects[i]))
        panic("shkveg probtype error, oclass=%d i=%d", (int) oclass, i);
    return i;
}

/**
 * @brief Place one piece of health food store stock at a spot.
 * @param sx column
 * @param sy row
 * @note A tin gets its contents set explicitly to the healthy variety. Without that a tin chosen as vegetarian stock would have its contents rolled at random when opened, and could turn out to hold meat -- so the assumption
 *       veggy_item() makes about types is corrected here, at the moment the object becomes real.
 */
/**
 * @brief 건강식품점 재고 한 점을 어떤 자리에 놓는다.
 * @param sx 열
 * @param sy 행
 * @note 통조림은 내용물이 명시적으로 건강한 종류로 설정된다. 그러지 않으면 채식 재고로 골라진 통조림이 열릴 때 내용물이 무작위로 굴려지고 고기를 담은 것으로 밝혀질 수 있다. 그래서 veggy_item()이 종류에 대해 하는 가정이, 물건이 실재하게 되는 순간인 여기서 바로잡힌다.
 */
staticfn void
mkveggy_at(int sx, int sy)
{
    struct obj *obj = mksobj_at(shkveg(), sx, sy, TRUE, TRUE);

    if (obj && obj->otyp == TIN)
        set_tin_variety(obj, HEALTHY_TIN);
    return;
}

/**
 * @brief Fill one square of a shop with something appropriate.
 *
 * "Appropriate" has three answers, tried in order. A bookstore's designated special square gets a novel. Failing that, the square may hold a mimic pretending to be stock, which is likelier the deeper the shop is. Otherwise it gets an
 * ordinary item drawn from the shop's own proportions.
 *
 * @param shp which kind of shop this is
 * @param sx column
 * @param sy row
 * @param mkspecl whether this is the square reserved for a special item
 * @note The mimic is not stock. It is placed instead of an object, so a shop with a mimic in it has one fewer thing to buy -- which is the point: the square looks occupied.
 * @note The special-square case matches the shop by its name text. That works but couples this to the wording in the shop table, so renaming a bookstore there silently stops novels appearing in it.
 * @note A negative item code means a specific type rather than a class, which is the convention the shop table's comment describes. The sign is the whole distinction, so a class value that ever went negative would be made as an object
 *       type with that number.
 */
/**
 * @brief 상점의 한 칸을 알맞은 무엇으로 채운다.
 *
 * "알맞은"에는 세 답이 있고, 차례로 시도된다. 서점의 지정된 특별 칸은 소설을 얻는다. 그것이 아니면 그 칸은 재고인 척하는 흉내쟁이를 담을 수 있고, 상점이 깊을수록 그럴 가능성이 커진다. 그것도 아니면 상점 자기 비율에서 뽑은 보통 물건을 얻는다.
 *
 * @param shp 이것이 어떤 종류의 상점인지
 * @param sx 열
 * @param sy 행
 * @param mkspecl 이것이 특별한 물건을 위해 남겨진 칸인지
 * @note 흉내쟁이는 재고가 아니다. 그것은 물건 대신 놓이므로, 흉내쟁이가 있는 상점은 살 것이 하나 적다. 그것이 요점이다. 그 칸은 차 있어 보인다.
 * @note 특별 칸의 경우는 상점을 이름 글로 맞춘다. 그것은 동작하지만 이것을 상점 표의 표현에 얽매므로, 거기서 서점의 이름을 바꾸면 조용히 소설이 그 안에 나타나기를 그친다.
 * @note 음수 물건 부호는 부류가 아니라 특정 종류를 뜻하며, 그것이 상점 표의 주석이 서술하는 관례다. 부호가 구분 전부이므로, 언젠가 음수가 된 부류 값은 그 번호의 물건 종류로 만들어질 것이다.
 */
staticfn void
mkshobj_at(const struct shclass *shp, int sx, int sy, boolean mkspecl)
{
    struct monst *mtmp;
    struct permonst *ptr;
    int atype;

    /* 3.6 tribute */
    if (mkspecl && (!strcmp(shp->name, "rare books")
                    || !strcmp(shp->name, "second-hand bookstore"))) {
        struct obj *novel = mksobj_at(SPE_NOVEL, sx, sy, FALSE, FALSE);

        if (novel)
            svc.context.tribute.bookstock = TRUE;
        return;
    }

    if (rn2(100) < depth(&u.uz) && !MON_AT(sx, sy)
        && (ptr = mkclass(S_MIMIC, 0)) != 0
        && (mtmp = makemon(ptr, sx, sy, NO_MM_FLAGS)) != 0) {
        /* nothing */
    } else {
        atype = get_shop_item((int) (shp - shtypes));
        if (atype == VEGETARIAN_CLASS)
            mkveggy_at(sx, sy);
        else if (atype < 0)
            (void) mksobj_at(-atype, sx, sy, TRUE, TRUE);
        else
            (void) mkobj_at(atype, sx, sy, TRUE);
    }
}

/**
 * @brief Give a shopkeeper a name from the list belonging to its kind of shop.
 *
 * The name is derived rather than drawn at random, and the existing comment explains why: a random name would have to be saved and restored, whereas one computed from things the game already remembers costs nothing to keep. The
 * ingredients are the shopkeeper's identity, which level it is on, and the moment this character was born -- so the same shop is named the same throughout a game and differently in the next one.
 *
 * @param shk the shopkeeper
 * @param nlp the list for this shop's kind
 * @note Minetown's lighting shop is named outright rather than derived. It is a fixed piece of the game that players recognise, so it cannot be left to a derivation.
 * @note The level is identified by its ledger number rather than its depth, as the existing comment notes, so that Minetown's shops are named differently from shops at the same depth elsewhere.
 * @note A leading character on each list entry says whether the name is male or female and whether it is a personal name, per the convention documented above the lists. It is stored as part of the name and stripped when the name is
 *       shown, so the gender set from the derivation is overridden by the entry actually chosen.
 * @note Names in use elsewhere on the level are rejected and another is tried, up to fifty times. Two shopkeepers with one name would be a genuine problem -- a player cannot tell which one they owe.
 * @note Tool shops draw at random rather than by derivation, so a tool shop's keeper is not stable across a game the way others are.
 * @warning Falls back to the general list, and finally to one of two fixed names, if the shop's own list cannot supply an unused one. That is a real possibility on a level with several shops of one kind, and the fixed names are why such
 *          a level does not end up with a nameless shopkeeper.
 */
/**
 * @brief 상점 주인에게 그 상점 종류에 속한 목록에서 이름을 준다.
 *
 * 이름은 무작위로 뽑히는 대신 유도되며, 기존 주석이 그 이유를 설명한다. 무작위 이름은 저장되고 복원되어야 하겠지만, 게임이 이미 기억하는 것들로부터 계산된 이름은 지니는 데 아무 비용이 들지 않는다. 재료는 주인의 정체, 그것이 어느 층에 있는지, 그리고 이 인물이 태어난 순간이다. 그래서 같은 상점이 한 게임 내내 같게 이름 지어지고 다음 게임에서는 다르게 이름 지어진다.
 *
 * @param shk 상점 주인
 * @param nlp 이 상점 종류를 위한 목록
 * @note 광산 마을의 조명 가게는 유도되는 대신 곧바로 이름 지어진다. 그것은 플레이어들이 알아보는 게임의 고정된 조각이므로, 유도에 맡겨질 수 없다.
 * @note 기존 주석이 적듯이 층은 깊이가 아니라 원장 번호로 식별되어, 광산 마을의 상점들이 다른 곳의 같은 깊이 상점들과 다르게 이름 지어진다.
 * @note 각 목록 항목의 앞 문자가 그 이름이 남성인지 여성인지, 그리고 개인 이름인지를 말한다. 목록 위에 문서화된 관례에 따른 것이다. 그것은 이름의 일부로 저장되고 이름이 보일 때 떼어진다. 그래서 유도에서 정해진 성별이 실제로 골라진 항목에 의해 덮어쓰인다.
 * @note 그 층의 다른 곳에서 쓰이고 있는 이름은 거부되고 다른 것이 시도되며, 오십 번까지다. 한 이름을 가진 두 주인은 진짜 문제일 것이다. 플레이어가 자기가 어느 쪽에게 빚졌는지 알 수 없다.
 * @note 도구 상점은 유도가 아니라 무작위로 뽑으므로, 도구 상점 주인은 다른 것들처럼 한 게임에 걸쳐 안정적이지 않다.
 * @warning 상점 자기 목록이 쓰이지 않은 이름을 내놓을 수 없으면 일반 목록으로, 끝내는 고정된 두 이름 중 하나로 물러난다. 그것은 한 종류의 상점이 여럿 있는 층에서 실제로 있을 수 있는 일이며, 그 고정된 이름들이 그런 층이 이름 없는 상점 주인으로 끝나지 않는 이유다.
 */
staticfn void
nameshk(struct monst *shk, const char *const *nlp)
{
    int i, trycnt, names_avail;
    const char *shname = 0;
    struct monst *mtmp;
    int name_wanted = shk->m_id;
    s_level *sptr;

    if (nlp == shklight && In_mines(&u.uz)
        && (sptr = Is_special(&u.uz)) != 0 && sptr->flags.town) {
        /* special-case minetown lighting shk */
        shname = "+Izchak";
        shk->female = FALSE;
    } else {
        /* We want variation from game to game, without needing the save
           and restore support which would be necessary for randomization;
           try not to make too many assumptions about time_t's internals;
           use ledger_no rather than depth to keep minetown distinct. */
        int nseed = (int) ((long) ubirthday / 257L);

        name_wanted += ledger_no(&u.uz) + (nseed % 13) - (nseed % 5);
        if (name_wanted < 0)
            name_wanted += (13 + 5);
        shk->female = name_wanted & 1;

        for (names_avail = 0; nlp[names_avail]; names_avail++)
            continue;
        assert(names_avail > 0);
        name_wanted = name_wanted % names_avail;

        for (trycnt = 0; trycnt < 50; trycnt++) {
            if (nlp == shktools) {
                shname = shktools[rn2(names_avail)];
                shk->female = 0; /* reversed below for '_' prefix */
            } else if (name_wanted < names_avail) {
                shname = nlp[name_wanted];
            } else if ((i = rn2(names_avail)) != 0) {
                shname = nlp[i - 1];
            } else if (nlp != shkgeneral) {
                nlp = shkgeneral; /* try general names */
                for (names_avail = 0; nlp[names_avail]; names_avail++)
                    continue;
                continue; /* next `trycnt' iteration */
            } else {
                shname = shk->female ? "-Lucrezia" : "+Dirk";
            }
            if (*shname == '_' || *shname == '-')
                shk->female = 1;
            else if (*shname == '|' || *shname == '+')
                shk->female = 0;

            /* is name already in use on this level? */
            for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
                if (DEADMONSTER(mtmp) || (mtmp == shk) || !mtmp->isshk)
                    continue;
                assert(has_eshk(mtmp));
                if (strcmp(ESHK(mtmp)->shknam, shname))
                    continue;
                name_wanted = names_avail; /* try a random name */
                break;
            }
            if (!mtmp)
                break; /* new name */
        }
    }
    (void) strncpy(ESHK(shk)->shknam, shname, PL_NSIZ);
    ESHK(shk)->shknam[PL_NSIZ - 1] = 0;
}

/**
 * @brief Give a monster the extra data a shopkeeper needs, cleared.
 * @param mtmp the monster becoming a shopkeeper
 * @note Allocates the general extra-data block first if the monster has none, because shopkeeper data lives inside it rather than beside it.
 * @note Everything is zeroed, and the bill pointer is set to none explicitly even though zeroing has already done it. Being explicit here says that a shopkeeper starts owed nothing, which is a fact about the game rather than an
 *       accident of allocation.
 * @note Records the monster's own identity as the parent. That is what lets the shopkeeper data be recognised as belonging to this monster after a save and restore, when pointers no longer mean anything.
 * @warning Does nothing if the data already exists except clear it -- so calling this on an established shopkeeper discards their bill and their memory of the hero without any indication.
 */
/**
 * @brief 몬스터에게 상점 주인이 필요한 추가 데이터를 지워진 채로 준다.
 * @param mtmp 상점 주인이 되는 몬스터
 * @note 몬스터가 아무것도 갖지 않으면 일반 추가 데이터 덩이를 먼저 할당한다. 상점 주인 데이터가 그 옆이 아니라 그 안에 살기 때문이다.
 * @note 모든 것이 영으로 되고, 영으로 만들기가 이미 그것을 했음에도 청구서 포인터가 명시적으로 없음으로 설정된다. 여기서 명시적인 것은, 상점 주인이 받을 것 없이 시작한다고 말하는 것이며, 그것은 할당의 우연이 아니라 게임에 관한 사실이다.
 * @note 몬스터 자신의 정체를 부모로 기록한다. 그것이 저장과 복원 뒤에, 포인터가 더는 아무것도 뜻하지 않을 때, 상점 주인 데이터가 이 몬스터의 것으로 알아보아지게 하는 것이다.
 * @warning 데이터가 이미 존재하면 그것을 지우는 것 말고는 아무것도 하지 않는다. 그래서 자리 잡은 상점 주인에게 이것을 호출하면 아무 표시 없이 그의 청구서와 영웅에 대한 기억을 버린다.
 */
void
neweshk(struct monst *mtmp)
{
    if (!mtmp->mextra)
        mtmp->mextra = newmextra();
    if (!ESHK(mtmp))
        ESHK(mtmp) = (struct eshk *) alloc(sizeof(struct eshk));
    (void) memset((genericptr_t) ESHK(mtmp), 0, sizeof(struct eshk));
    ESHK(mtmp)->parentmid = mtmp->m_id;
    ESHK(mtmp)->bill_p = (struct bill_x *) 0;
}

/**
 * @brief Release a monster's shopkeeper data and stop it being a shopkeeper.
 * @param mtmp the monster
 * @note Clears the is-a-shopkeeper flag whether or not there was data to free. The flag and the data must not disagree: code elsewhere reads the flag and then dereferences the data without checking, so a monster flagged as a shopkeeper
 *       with no data would be dereferenced through nothing.
 * @warning Does not touch the bill. A shopkeeper with an outstanding bill loses the record of it here, which is right only because this is called when the shopkeeper ceases to exist as one.
 */
/**
 * @brief 몬스터의 상점 주인 데이터를 해제하고 그것이 상점 주인이기를 그치게 한다.
 * @param mtmp 그 몬스터
 * @note 해제할 데이터가 있었든 없었든 상점 주인 표시를 지운다. 표시와 데이터가 어긋나서는 안 된다. 다른 곳의 코드가 그 표시를 읽고 검사 없이 데이터를 따라가므로, 데이터 없이 상점 주인으로 표시된 몬스터는 아무것도 아닌 것을 통해 따라가질 것이다.
 * @warning 청구서를 건드리지 않는다. 미결 청구서가 있는 상점 주인은 여기서 그것의 기록을 잃는데, 그것이 옳은 것은 오직 이것이 그 주인이 주인으로서 존재하기를 그칠 때 호출되기 때문이다.
 */
void
free_eshk(struct monst *mtmp)
{
    if (mtmp->mextra && ESHK(mtmp)) {
        free((genericptr_t) ESHK(mtmp));
        ESHK(mtmp) = (struct eshk *) 0;
    }
    mtmp->isshk = 0;
}

/**
 * @brief Find a door this room can use as a shop entrance, and the square inside it where the shopkeeper stands.
 *
 * The shopkeeper stands beside the door, not in it, because standing in the doorway would block the only way in. So this does two things at once: it picks a door and it steps one square inward from it. A door it cannot step inward from
 * is not usable, which is what makes some rooms unable to be shops.
 *
 * @param sroom the room
 * @param sx receives the column the shopkeeper will stand on
 * @param sy receives the row
 * @return the door's index, or -1 if the room has no usable door
 * @note For a rectangular room, stepping inward is a matter of knowing which wall the door is in. For an irregular room it is not, so each of the four neighbours is tried and accepted only if it is inside this same room and not on its
 *       edge -- an irregular room's inside cannot be worked out from its bounds.
 * @note The coordinates are written even for doors that are then rejected, so a caller that ignores a -1 return reads a position belonging to no usable door.
 */
/**
 * @brief 이 방이 상점 입구로 쓸 수 있는 문과, 그 안쪽에서 주인이 서는 칸을 찾는다.
 *
 * 주인은 문 안이 아니라 문 옆에 선다. 문간에 서면 들어오는 유일한 길을 막을 것이기 때문이다. 그래서 이것은 한 번에 두 가지를 한다. 문을 고르고, 그것에서 한 칸 안으로 들어간다. 안으로 들어갈 수 없는 문은 쓸 수 없는 문이며, 그것이 어떤 방들이 상점이 될 수 없게 만드는 것이다.
 *
 * @param sroom 그 방
 * @param sx 주인이 설 열을 받는다
 * @param sy 그 행을 받는다
 * @return 문의 색인, 또는 방에 쓸 수 있는 문이 없으면 -1
 * @note 직사각형 방에서 안으로 들어가는 것은 문이 어느 벽에 있는지 아는 문제다. 불규칙한 방에서는 그렇지 않으므로, 네 이웃 각각이 시도되고 그것이 같은 이 방 안에 있으며 그 가장자리에 없을 때만 받아들여진다. 불규칙한 방의 안쪽은 그 경계에서 알아낼 수 없다.
 * @note 좌표는 그다음 거부되는 문에 대해서도 쓰이므로, -1 반환을 무시하는 호출자는 쓸 수 있는 어떤 문에도 속하지 않는 위치를 읽는다.
 */
staticfn int
good_shopdoor(struct mkroom *sroom, coordxy *sx, coordxy *sy)
{
    int i;

    for (i = 0; i < sroom->doorct; i++) {
        int di = sroom->fdoor + i;

        *sx = svd.doors[di].x;
        *sy = svd.doors[di].y;

        /* check that the shopkeeper placement is sane */
        if (sroom->irregular) {
            int rmno = (int) ((sroom - svr.rooms) + ROOMOFFSET);

            if (isok(*sx - 1, *sy) && !levl[*sx - 1][*sy].edge
                && (int) levl[*sx - 1][*sy].roomno == rmno)
                (*sx)--;
            else if (isok(*sx + 1, *sy) && !levl[*sx + 1][*sy].edge
                     && (int) levl[*sx + 1][*sy].roomno == rmno)
                (*sx)++;
            else if (isok(*sx, *sy - 1) && !levl[*sx][*sy - 1].edge
                     && (int) levl[*sx][*sy - 1].roomno == rmno)
                (*sy)--;
            else if (isok(*sx, *sy + 1) && !levl[*sx][*sy + 1].edge
                     && (int) levl[*sx][*sy + 1].roomno == rmno)
                (*sy)++;
            else
                continue;
        } else if (*sx == sroom->lx - 1) {
            (*sx)++;
        } else if (*sx == sroom->hx + 1) {
            (*sx)--;
        } else if (*sy == sroom->ly - 1) {
            (*sy)++;
        } else if (*sy == sroom->hy + 1) {
            (*sy)--;
        } else {
            continue;
        }
        return di;
    }
    return -1;
}

/**
 * @brief Create the shopkeeper for a room and make the room theirs.
 *
 * The point where a room and a monster become a shop. The room records who lives in it and the shopkeeper records which room they keep, on which level, and where their door is -- all four, because the shopkeeper must be able to tell
 * whether the hero is inside their shop from anywhere, including from another level.
 *
 * @param shp which kind of shop
 * @param sroom the room
 * @return the door index, or -1 if a shopkeeper could not be made
 * @note A monster already standing where the shopkeeper must go is moved elsewhere, described in the code as insurance. It is not optional: the shopkeeper's position is not negotiable once the door is chosen.
 * @note The shopkeeper is given money to start with, which is their capital rather than treasure. It is what lets them buy from the hero, so a shopkeeper without it could only sell.
 * @note Told about every trap on the level. A shopkeeper who stepped into a trap while chasing a thief would be a shopkeeper the hero could rob by walking over one.
 * @note Certain kinds get a tool of their trade -- a touchstone for a ring shop, a charging scroll for shops selling things with charges. That is the shopkeeper being equipped to do business in what they sell, and the ring shop's
 *       touchstone is why a ring shop can appraise.
 * @warning Returning -1 leaves the room not a shop. The caller must not stock it, because stocked goods with no shopkeeper are free.
 */
/**
 * @brief 어떤 방을 위한 상점 주인을 만들고 그 방을 그의 것으로 만든다.
 *
 * 방과 몬스터가 상점이 되는 지점. 방은 누가 그 안에 사는지 기록하고, 주인은 자기가 어느 방을 지키는지, 어느 층에서인지, 자기 문이 어디인지를 기록한다. 넷 모두다. 주인이 어디서든, 다른 층에서라도, 영웅이 자기 상점 안에 있는지 말할 수 있어야 하기 때문이다.
 *
 * @param shp 어떤 종류의 상점인지
 * @param sroom 그 방
 * @return 문 색인, 또는 주인을 만들 수 없었으면 -1
 * @note 주인이 가야 하는 곳에 이미 서 있는 몬스터는 다른 곳으로 옮겨지며, 코드에서 보험이라고 서술된다. 그것은 선택 사항이 아니다. 문이 골라진 뒤 주인의 위치는 협상할 수 없다.
 * @note 주인은 시작할 돈을 받는데, 그것은 보물이 아니라 자본이다. 그것이 그가 영웅에게서 살 수 있게 하는 것이며, 그래서 그것 없는 주인은 팔기만 할 수 있을 것이다.
 * @note 그 층의 모든 함정에 대해 들어 안다. 도둑을 쫓다가 함정에 밟혀 들어갈 주인은 영웅이 함정 위를 걸어감으로써 털 수 있는 주인일 것이다.
 * @note 어떤 종류들은 자기 장사의 도구를 받는다. 반지 상점에는 시금석, 충전량이 있는 것을 파는 상점에는 충전 주문서. 그것은 주인이 자기가 파는 것으로 장사할 수 있도록 갖추어지는 것이며, 반지 상점의 시금석이 반지 상점이 감정할 수 있는 이유다.
 * @warning -1을 돌려주는 것은 그 방을 상점이 아닌 채로 남긴다. 호출자는 그것에 물건을 채워서는 안 된다. 주인 없는 채워진 상품은 임자 없는 것이기 때문이다.
 */
staticfn int
shkinit(const struct shclass *shp, struct mkroom *sroom)
{
    int sh;
    coordxy sx, sy;
    struct monst *shk;
    struct eshk *eshkp;

    /* place the shopkeeper in the given room */
    sh = good_shopdoor(sroom, &sx, &sy);
    if (sh < 0) {
#ifdef DEBUG
        /* Said to happen sometimes, but I have never seen it. */
        /* Supposedly fixed by fdoor change in mklev.c */
        if (wizard) {
            int j = sroom->doorct;

            impossible("Where is shopdoor?");
            pline("Room at (%d,%d),(%d,%d).", sroom->lx, sroom->ly, sroom->hx,
                  sroom->hy);
            pline("doormax=%d doorct=%d fdoor=%d", gd.doorindex, sroom->doorct,
                  sh);
            while (j--) {
                pline("door [%d,%d]", svd.doors[sh].x, svd.doors[sh].y);
                sh++;
            }
            display_nhwindow(WIN_MESSAGE, FALSE);
        }
#endif
        return -1;
    }

    if (MON_AT(sx, sy))
        (void) rloc(m_at(sx, sy), RLOC_NOMSG); /* insurance */

    /* now initialize the shopkeeper monster structure */
    if (!(shk = makemon(&mons[PM_SHOPKEEPER], sx, sy, MM_ESHK)))
        return -1;
    eshkp = ESHK(shk); /* makemon(...,MM_ESHK) allocates this */
    shk->isshk = shk->mpeaceful = 1;
    set_malign(shk);
    shk->msleeping = 0;
    mon_learns_traps(shk, ALL_TRAPS); /* we know all the traps already */
    eshkp->shoproom = (schar) ((sroom - svr.rooms) + ROOMOFFSET);
    sroom->resident = shk;
    eshkp->shoptype = sroom->rtype;
    assign_level(&eshkp->shoplevel, &u.uz);
    eshkp->shd = svd.doors[sh];
    eshkp->shk.x = sx;
    eshkp->shk.y = sy;
    eshkp->robbed = eshkp->credit = eshkp->debit = eshkp->loan = 0L;
    eshkp->following = eshkp->surcharge = eshkp->dismiss_kops = FALSE;
    eshkp->billct = eshkp->visitct = 0;
    eshkp->bill_p = (struct bill_x *) 0;
    eshkp->customer[0] = '\0';
    mkmonmoney(shk, 1000L + 30L * (long) rnd(100)); /* initial capital */
    if (shp->shknms == shkrings)
        (void) mongets(shk, TOUCHSTONE);
    if (shp->shknms == shktools || shp->shknms == shkwands ||
        (shp->shknms == shkrings && rn2(2)) ||
        (shp->shknms == shkgeneral && rn2(5)))
        (void) mongets(shk, SCR_CHARGING);
    nameshk(shk, shp->shknms);

    return sh;
}

/**
 * @brief May this square hold stock?
 *
 * The squares nearest the door are deliberately left empty. That is not decoration: it gives the hero somewhere to stand on entering, and gives the shopkeeper room to step aside. A shop stocked right up to its door would trap the hero
 * into picking something up merely by walking in.
 *
 * @param sroom the room
 * @param rmno the room's number
 * @param sh the shop door's index
 * @param sx column
 * @param sy row
 * @return whether stock may go here
 * @note The two shapes of room need different tests. A rectangular room's door-adjacent squares are found by comparing against the room's bounds; an irregular room's are found by distance from the door, and it must also be checked that
 *       the square belongs to this room at all, since an irregular room's bounds enclose squares that are not part of it.
 * @note Only ordinary floor may hold stock. A shop room can contain other terrain, and an object on it might be unreachable, which would be a thing on the bill the hero cannot pay for or return.
 */
/**
 * @brief 이 칸이 재고를 담을 수 있는가?
 *
 * 문에 가장 가까운 칸들은 의도적으로 비워진다. 그것은 장식이 아니다. 그것은 들어오는 영웅에게 설 곳을 주고, 주인에게 비켜설 자리를 준다. 문까지 빽빽하게 채워진 상점은 그저 걸어 들어오기만 해도 영웅이 무언가를 집게 만들 것이다.
 *
 * @param sroom 그 방
 * @param rmno 그 방의 번호
 * @param sh 상점 문의 색인
 * @param sx 열
 * @param sy 행
 * @return 재고가 여기 놓일 수 있는지
 * @note 두 모양의 방이 다른 검사를 필요로 한다. 직사각형 방의 문 인접 칸들은 방의 경계와 비교해 찾아지고, 불규칙한 방의 것들은 문으로부터의 거리로 찾아지며, 그 칸이 이 방에 속하기는 하는지도 검사되어야 한다. 불규칙한 방의 경계는 그것의 일부가 아닌 칸들을 감싸기 때문이다.
 * @note 보통 바닥만 재고를 담을 수 있다. 상점 방은 다른 지형을 담을 수 있고, 그 위의 물건은 닿을 수 없을지도 모른다. 그것은 영웅이 값을 치를 수도 되돌릴 수도 없는 청구서 위의 물건이 될 것이다.
 */
staticfn boolean
stock_room_goodpos(struct mkroom *sroom, int rmno, int sh, int sx, int sy)
{
    if (sroom->irregular) {
        if (levl[sx][sy].edge
            || (int) levl[sx][sy].roomno != rmno
            || distmin(sx, sy, svd.doors[sh].x, svd.doors[sh].y) <= 1)
            return FALSE;
    } else if ((sx == sroom->lx && svd.doors[sh].x == sx - 1)
               || (sx == sroom->hx && svd.doors[sh].x == sx + 1)
               || (sy == sroom->ly && svd.doors[sh].y == sy - 1)
               || (sy == sroom->hy && svd.doors[sh].y == sy + 1))
        return FALSE;

    /* only generate items on solid floor squares */
    if (!IS_ROOM(levl[sx][sy].typ)) {
        return FALSE;
    }

    return TRUE;
}

/**
 * @brief Turn a room into a working shop: shopkeeper, door, and goods.
 *
 * The shopkeeper comes first and everything else depends on it -- if one cannot be made the room is left alone, because goods without an owner are goods the hero may simply take.
 *
 * Then the door is made fit for a shop. A shop must have a real door that is not trapped, so a doorless doorway is given an open door, a hidden door is revealed, and a trapped door is merely locked instead. None of these are cosmetic:
 * a shop the hero can enter without passing a door is a shop they can rob, and a trap on a shop door would injure the shopkeeper's own customers.
 *
 * Finally every square that may hold stock gets something.
 *
 * @param shp_indx which kind of shop
 * @param sroom the room
 * @note A locked shop door gets "Closed for inventory" written on the floor beside it, inside. It is the explanation for a door the hero cannot open, written where they will find it once they get in.
 * @note The tribute bookstore's special square is chosen by counting the stockable squares first and then filling them a second time, because a square cannot be singled out at random until it is known how many there are.
 * @note Orcus's level removes the shopkeeper again immediately afterwards, described in the code as a hack. The goods stay, which is the intent -- it is a ghost town, and abandoned stock is the point.
 * @note The placement style is fixed. As the existing comment concedes, the shop-kind table has a field intended to select among styles someday and only the one style exists.
 */
/**
 * @brief 방을 작동하는 상점으로 만든다. 주인, 문, 그리고 상품.
 *
 * 주인이 먼저 오고 나머지 모두가 그것에 달려 있다. 주인을 만들 수 없으면 방은 그대로 남겨진다. 임자 없는 상품은 영웅이 그냥 가져갈 수 있는 상품이기 때문이다.
 *
 * 그다음 문이 상점에 맞게 만들어진다. 상점은 함정이 걸리지 않은 진짜 문을 가져야 하므로, 문 없는 문간은 열린 문을 받고, 숨겨진 문은 드러나며, 함정이 걸린 문은 그저 잠기는 것으로 대신된다. 이 가운데 어느 것도 겉모습이 아니다. 영웅이 문을 지나지 않고 들어갈 수 있는 상점은 그가 털 수 있는 상점이며, 상점 문의 함정은 주인 자신의 손님을 다치게 할 것이다.
 *
 * 마지막으로 재고를 담을 수 있는 모든 칸이 무언가를 받는다.
 *
 * @param shp_indx 어떤 종류의 상점인지
 * @param sroom 그 방
 * @note 잠긴 상점 문은 그 옆 안쪽 바닥에 "재고 정리 중 휴업"이 쓰인다. 그것은 영웅이 열 수 없는 문에 대한 설명이며, 그가 들어가고 나면 발견할 곳에 쓰인다.
 * @note 헌정 서점의 특별한 칸은 재고를 놓을 수 있는 칸을 먼저 세고 그다음 그것들을 두 번째로 채우면서 골라진다. 몇 개가 있는지 알려지기 전에는 어떤 칸도 무작위로 지목될 수 없기 때문이다.
 * @note 오르쿠스의 층은 바로 뒤에 주인을 다시 없애며, 코드에서 임시 방편이라고 서술된다. 상품은 남고, 그것이 의도다. 그것은 유령 마을이며, 버려진 재고가 요점이다.
 * @note 배치 방식은 고정되어 있다. 기존 주석이 인정하듯, 상점 종류 표에는 언젠가 방식들 중에서 고르려던 항목이 있고 그 한 방식만 존재한다.
 */
void
stock_room(int shp_indx, struct mkroom *sroom)
{
    /*
     * Someday soon we'll dispatch on the shdist field of shclass to do
     * different placements in this routine. Currently it only supports
     * shop-style placement (all squares except a row nearest the first
     * door get objects).
     */
    int sx, sy, sh;
    int stockcount = 0, specialspot = 0;
    char buf[BUFSZ];
    int rmno = (int) ((sroom - svr.rooms) + ROOMOFFSET);
    const struct shclass *shp = &shtypes[shp_indx];

    /* first, try to place a shopkeeper in the room */
    if ((sh = shkinit(shp, sroom)) < 0)
        return;

    /* make sure no doorways without doors, and no trapped doors, in shops */
    sx = svd.doors[sroom->fdoor].x;
    sy = svd.doors[sroom->fdoor].y;
    if (levl[sx][sy].doormask == D_NODOOR) {
        levl[sx][sy].doormask = D_ISOPEN;
        newsym(sx, sy);
    }
    if (levl[sx][sy].typ == SDOOR) {
        cvt_sdoor_to_door(&levl[sx][sy]); /* .typ = DOOR */
        newsym(sx, sy);
    }
    if (levl[sx][sy].doormask & D_TRAPPED)
        levl[sx][sy].doormask = D_LOCKED;

    if (levl[sx][sy].doormask == D_LOCKED) {
        int m = sx, n = sy;

        if (inside_shop(sx + 1, sy))
            m--;
        else if (inside_shop(sx - 1, sy))
            m++;
        if (inside_shop(sx, sy + 1))
            n--;
        else if (inside_shop(sx, sy - 1))
            n++;
        Sprintf(buf, "Closed for inventory");
        make_engr_at(m, n, buf, NULL, 0L, DUST);
        if (levl[m][n].typ != CORR && levl[m][n].typ != ROOM)
            levl[m][n].typ = (Is_special(&u.uz)
                              || *in_rooms(m, n, 0)) ? ROOM : CORR;
    }

    if (svc.context.tribute.enabled && !svc.context.tribute.bookstock) {
        /*
         * Out of the number of spots where we're actually
         * going to put stuff, randomly single out one in particular.
         */
        for (sx = sroom->lx; sx <= sroom->hx; sx++)
            for (sy = sroom->ly; sy <= sroom->hy; sy++)
                if (stock_room_goodpos(sroom, rmno, sh, sx,sy))
                    stockcount++;
        specialspot = rnd(stockcount);
        stockcount = 0;
    }

    for (sx = sroom->lx; sx <= sroom->hx; sx++)
        for (sy = sroom->ly; sy <= sroom->hy; sy++)
            if (stock_room_goodpos(sroom, rmno, sh, sx,sy)) {
                stockcount++;
                mkshobj_at(shp, sx, sy,
                           ((stockcount) && (stockcount == specialspot)));
            }

    /*
     * Special monster placements (if any) should go here: that way,
     * monsters will sit on top of objects and not the other way around.
     */

    /* Hack for Orcus's level: it's a ghost town, get rid of shopkeepers */
    if (on_level(&u.uz, &orcus_level)) {
        struct monst *mtmp = shop_keeper(rmno);
        mongone(mtmp);
    }

    svl.level.flags.has_shop = TRUE;
}

/**
 * @brief Would this shopkeeper buy this object?
 *
 * Asked when the hero drops something in a shop, and the answer is the difference between being paid and being told to take it away. It is decided from the same table that decided what to stock the shop with, so a shop buys precisely
 * what it could have been stocked with.
 *
 * @param shkp the shopkeeper
 * @param obj the object
 * @return whether the shop deals in it
 * @note A general store buys everything, tested first because its table entry says only "anything" and walking the entry would not reveal that.
 * @note The vegetarian pseudo-class cannot be compared like a real class, so it is dispatched to veggy_item() instead. The object exists here, so unlike at stocking time the doubtful cases are decided rather than assumed.
 * @note Sign distinguishes a specific type from a class, the same convention used when stocking. Reading a negative entry as a class would compare an object class against a negated object type, and small numbers on both sides make
 *       accidental matches plausible.
 * @warning Assumes the monster is a shopkeeper with shopkeeper data; it dereferences that data to find the shop's kind without checking.
 */
/**
 * @brief 이 상점 주인이 이 물건을 사겠는가?
 *
 * 영웅이 상점에서 무언가를 내려놓을 때 물어지며, 그 답은 값을 받는 것과 그것을 치우라고 듣는 것의 차이다. 그 상점에 무엇을 채울지 정한 것과 같은 표에서 결정되므로, 상점은 자기가 채워질 수 있었던 것을 정확히 산다.
 *
 * @param shkp 상점 주인
 * @param obj 그 물건
 * @return 상점이 그것을 다루는지
 * @note 일반 상점은 모든 것을 산다. 그것의 표 항목이 "무엇이든"만 말하고 그 항목을 걸어가는 것이 그것을 드러내지 않을 것이므로 먼저 검사된다.
 * @note 채식 가짜 부류는 진짜 부류처럼 비교될 수 없으므로 대신 veggy_item()으로 보내진다. 물건이 여기 존재하므로, 물건을 채울 때와 달리 의심스러운 경우가 가정되는 대신 결정된다.
 * @note 부호가 특정 종류를 부류와 구별하며, 물건을 채울 때 쓰인 것과 같은 관례다. 음수 항목을 부류로 읽으면 물건 부류를 음수화된 물건 종류와 비교할 것이고, 양쪽의 작은 수들이 우연한 일치를 그럴듯하게 만든다.
 * @warning 그 몬스터가 상점 주인 데이터를 가진 상점 주인이라고 가정한다. 검사 없이 그 데이터를 따라가 상점의 종류를 찾는다.
 */
boolean
saleable(struct monst *shkp, struct obj *obj)
{
    int i, shp_indx = ESHK(shkp)->shoptype - SHOPBASE;
    const struct shclass *shp = &shtypes[shp_indx];

    if (shp->symb == RANDOM_CLASS)
        return TRUE;
    for (i = 0; i < SIZE(shtypes[0].iprobs) && shp->iprobs[i].iprob; i++) {
        /* pseudo-class needs special handling */
        if (shp->iprobs[i].itype == VEGETARIAN_CLASS) {
            if (veggy_item(obj, 0))
                return TRUE;
        } else if ((shp->iprobs[i].itype < 0)
                       ? shp->iprobs[i].itype == -obj->otyp
                       : shp->iprobs[i].itype == obj->oclass)
            return TRUE;
    }
    /* not found */
    return FALSE;
}

/**
 * @brief Draw one item code from a shop kind's proportions.
 * @param type which kind of shop
 * @return positive for an object class, negative for a specific object type, as the existing comment records
 * @warning The result may also be a pseudo-class that no object actually has -- the vegetarian one. As the existing comment warns, a caller that treats every positive value as a real object class will ask the object machinery for a class
 *          that does not exist. Callers must test for the pseudo-class before doing anything else with the value.
 * @note Walks the proportions until the drawn number is used up, with no bound on the walk. That is safe only while the proportions total a hundred, which is what the disabled init_shop_selection() existed to check.
 */
/**
 * @brief 어떤 상점 종류의 비율에서 물건 부호 하나를 뽑는다.
 * @param type 어떤 종류의 상점인지
 * @return 기존 주석이 기록하듯, 물건 부류에는 양수, 특정 물건 종류에는 음수
 * @warning 결과는 어떤 물건도 실제로 갖지 않는 가짜 부류일 수도 있다. 채식 부류다. 기존 주석이 경고하듯, 모든 양수 값을 진짜 물건 부류로 다루는 호출자는 존재하지 않는 부류를 물건 기계에 요청할 것이다. 호출자는 그 값으로 다른 무엇이든 하기 전에 가짜 부류를 검사해야 한다.
 * @note 뽑힌 수가 다할 때까지 비율을 걸어가며, 그 걸어가기에 한계가 없다. 그것은 비율이 백을 합할 동안만 안전하며, 그것이 비활성화된 init_shop_selection()이 검사하기 위해 존재했던 것이다.
 */
int
get_shop_item(int type)
{
    const struct shclass *shp = shtypes + type;
    int i, j;

    /* select an appropriate object type at random */
    for (j = rnd(100), i = 0; (j -= shp->iprobs[i].iprob) > 0; i++)
        continue;

    return shp->iprobs[i].itype;
}

/**
 * @brief The shopkeeper's name, capitalised for the start of a sentence.
 * @param mtmp the shopkeeper
 * @return the name
 * @note Almost always identical to shkname(), since names are stored capitalised. It exists anyway because "almost always" is not always -- the hallucinatory case and the fallbacks can produce something else -- and a sentence beginning
 *       in lower case is a visible defect.
 */
/**
 * @brief 상점 주인의 이름. 문장 시작을 위해 첫 글자가 대문자로.
 * @param mtmp 상점 주인
 * @return 그 이름
 * @note 이름이 대문자로 저장되므로 거의 언제나 shkname()과 똑같다. 그럼에도 존재하는 것은 "거의 언제나"가 언제나는 아니기 때문이다. 환각의 경우와 물러섬들이 다른 것을 내놓을 수 있다. 그리고 소문자로 시작하는 문장은 눈에 보이는 결함이다.
 */
char *
Shknam(struct monst *mtmp)
{
    char *nam = shkname(mtmp);

    /* 'nam[]' is almost certainly already capitalized, but be sure */
    nam[0] = highc(nam[0]);
    return nam;
}

/**
 * @brief The shopkeeper's name, regardless of whether the hero can see them.
 *
 * Deliberately not subject to visibility, as the existing comment records. The hero knows who keeps a shop they have been in, so hiding the name because the shopkeeper is momentarily out of sight would be withholding something the hero
 * already knows.
 *
 * @param mtmp the shopkeeper
 * @return the name, in a buffer that will be reused
 * @note Turns off the is-a-shopkeeper flag before asking the general naming machinery for a buffer, and restores it after. Without that the general machinery would call back here and the two would call each other without end.
 * @note Hallucination yields a different shopkeeper's name entirely, drawn from any shop kind's list. As the existing comment concedes, it need not be a name from this dungeon or from a shop of this type -- which is the intent, since a
 *       hallucinating hero is not reading anything reliably.
 * @note The hallucinatory draw ignores how likely each shop kind is to be generated, and stops at the first kind with no probability, because those are the unique shops and their keepers' names are not interchangeable.
 * @note The stored gender-and-kind prefix is stripped here. This is the point where a stored name becomes a displayable one, which is why nothing else needs to know about the prefix convention.
 * @note Complains rather than failing if the monster is not a shopkeeper, but ends the game if it is one with no shopkeeper data. The second is not recoverable: the name is in that data and there is nothing to fall back to.
 * @note Hallucination is suppressed once the game is over, so the end-of-game report names the shopkeeper who actually killed the hero rather than one the hero imagined.
 */
/**
 * @brief 영웅이 그를 볼 수 있는지와 무관한, 상점 주인의 이름.
 *
 * 기존 주석이 기록하듯 의도적으로 가시성에 좌우되지 않는다. 영웅은 자기가 들어가 본 상점을 누가 지키는지 안다. 그러므로 주인이 잠시 시야에서 벗어났다고 이름을 감추는 것은 영웅이 이미 아는 것을 감추는 일일 것이다.
 *
 * @param mtmp 상점 주인
 * @return 그 이름. 재사용될 버퍼에 담겨
 * @note 일반 이름 짓기 기계에 버퍼를 요청하기 전에 상점 주인 표시를 끄고, 뒤에 되돌린다. 그것 없으면 일반 기계가 여기로 되돌아 호출하고 그 둘이 끝없이 서로를 호출할 것이다.
 * @note 환각은 아예 다른 상점 주인의 이름을 내며, 어떤 상점 종류의 목록에서든 뽑힌다. 기존 주석이 인정하듯 그것은 이 던전의 이름일 필요도, 이 종류 상점의 이름일 필요도 없다. 그것이 의도다. 환각에 빠진 영웅은 무엇도 믿을 만하게 읽고 있지 않다.
 * @note 환각의 뽑기는 각 상점 종류가 생성될 가능성을 무시하며, 확률이 없는 첫 종류에서 멈춘다. 그것들이 유일한 상점들이고 그 주인의 이름은 서로 바꿔 쓸 수 있는 것이 아니기 때문이다.
 * @note 저장된 성별과 종류 접두 문자가 여기서 떼어진다. 이곳이 저장된 이름이 보일 수 있는 이름이 되는 지점이며, 그것이 다른 무엇도 그 접두 관례를 알 필요가 없는 이유다.
 * @note 그 몬스터가 상점 주인이 아니면 실패하는 대신 불평하지만, 상점 주인 데이터가 없는 상점 주인이면 게임을 끝낸다. 두 번째는 회복할 수 없다. 이름이 그 데이터 안에 있고 물러설 곳이 없다.
 * @note 게임이 끝나면 환각이 억제되므로, 종료 보고가 영웅이 상상한 주인이 아니라 실제로 영웅을 죽인 주인을 이름 짓는다.
 */
char *
shkname(struct monst *mtmp)
{
    char *nam;
    unsigned save_isshk = mtmp->isshk;

    mtmp->isshk = 0; /* don't want mon_nam() calling shkname() */
    /* get a modifiable name buffer along with fallback result */
    nam = noit_mon_nam(mtmp);
    mtmp->isshk = save_isshk;

    if (!mtmp->isshk) {
        impossible("shkname: \"%s\" is not a shopkeeper.", nam);
    } else if (!has_eshk(mtmp)) {
        panic("shkname: shopkeeper \"%s\" lacks 'eshk' data.", nam);
    } else {
        const char *shknm = ESHK(mtmp)->shknam;

        if (Hallucination && !program_state.gameover) {
            const char *const *nlp;
            int num;

            /* count the number of non-unique shop types;
               pick one randomly, ignoring shop generation probabilities;
               pick a name at random from that shop type's list */
            for (num = 0; num < SIZE(shtypes); num++)
                if (shtypes[num].prob == 0)
                    break;
            if (num > 0) {
                nlp = shtypes[rn2(num)].shknms;
                for (num = 0; nlp[num]; num++)
                    continue;
                if (num > 0)
                    shknm = nlp[rn2(num)];
            }
        }
        /* strip prefix if present */
        if (!letter(*shknm))
            ++shknm;
        Strcpy(nam, shknm);
    }
    return nam;
}

/**
 * @brief Is this shopkeeper's name a personal name?
 *
 * The distinction decides whether the name gets an honorific. A general name is used as "Mr. Prokop"; a personal name stands alone, because "Mr. Izchak" is not how a person with a given name is addressed.
 *
 * @param mtmp the shopkeeper
 * @return whether the name is personal
 * @note Read from the stored prefix character rather than from any separate flag, per the convention documented above the name lists. Three of the five prefixes mark personal names -- one for each gender and one for unspecified.
 * @warning Must be asked before the prefix is stripped. shkname() strips it, so this cannot be answered from what shkname() returns.
 */
/**
 * @brief 이 상점 주인의 이름이 개인 이름인가?
 *
 * 그 구별이 이름이 존칭을 받는지를 정한다. 일반 이름은 "프로코프 씨"로 쓰이고, 개인 이름은 홀로 선다. "이즈착 씨"는 이름을 가진 사람을 부르는 방식이 아니기 때문이다.
 *
 * @param mtmp 상점 주인
 * @return 그 이름이 개인 이름인지
 * @note 따로 된 표시가 아니라 저장된 접두 문자에서 읽는다. 이름 목록 위에 문서화된 관례에 따른 것이다. 다섯 접두 중 셋이 개인 이름을 표시한다. 성별마다 하나, 그리고 명시되지 않은 것 하나.
 * @warning 접두 문자가 떼어지기 전에 물어져야 한다. shkname()이 그것을 떼므로, 이것은 shkname()이 돌려주는 것에서 답할 수 없다.
 */
boolean
shkname_is_pname(struct monst *mtmp)
{
    const char *shknm = ESHK(mtmp)->shknam;

    return (boolean) (*shknm == '-' || *shknm == '+' || *shknm == '=');
}

/**
 * @brief Is this Izchak, the Minetown lighting shopkeeper?
 *
 * Asked because Izchak is treated specially -- he has his own dialogue and his own place in the game's lore. So this is not merely a name comparison: it is asking whether this monster is the character players know.
 *
 * @param shkp the shopkeeper
 * @param override_hallucination whether to answer truthfully even while hallucinating
 * @return whether this is Izchak
 * @note Outside town the answer is no even for a shopkeeper of that name, as the existing comment records. Izchak is a resident of Minetown; a shopkeeper who has left it is just a shopkeeper, and the special dialogue would be out of
 *       place.
 * @note Normally answers no while hallucinating, because the hero cannot recognise anyone reliably then. The override exists for callers that need the fact rather than the hero's impression of it -- deciding what actually happens must
 *       not depend on what the hero can perceive.
 * @note The prefix is skipped before comparing, as with the other name readers.
 */
/**
 * @brief 이것이 광산 마을 조명 가게 주인 이즈착인가?
 *
 * 이즈착이 특별히 다뤄지기 때문에 물어진다. 그는 자기만의 대화와 게임 설화 속 자기 자리를 갖는다. 그래서 이것은 단순한 이름 비교가 아니다. 이 몬스터가 플레이어들이 아는 그 인물인지를 묻는 것이다.
 *
 * @param shkp 상점 주인
 * @param override_hallucination 환각 중에도 참되게 답할지
 * @return 이것이 이즈착인지
 * @note 기존 주석이 기록하듯, 마을 밖에서는 그 이름의 주인에게조차 답이 아니오다. 이즈착은 광산 마을의 주민이다. 그곳을 떠난 주인은 그저 주인이며, 그 특별한 대화는 어울리지 않을 것이다.
 * @note 환각 중에는 보통 아니오라고 답한다. 그때 영웅은 누구도 믿을 만하게 알아볼 수 없기 때문이다. 그 무시 표시는 영웅의 인상이 아니라 사실을 필요로 하는 호출자를 위해 존재한다. 실제로 무엇이 일어나는지 정하는 것이 영웅이 지각할 수 있는 것에 좌우되어서는 안 된다.
 * @note 다른 이름 읽기들처럼 비교하기 전에 접두 문자가 건너뛰어진다.
 */
boolean
is_izchak(struct monst *shkp, boolean override_hallucination)
{
    const char *shknm;

    if (Hallucination && !override_hallucination)
        return FALSE;
    if (!shkp->isshk)
        return FALSE;
    /* outside of town, Izchak becomes just an ordinary shopkeeper */
    if (!in_town(shkp->mx, shkp->my))
        return FALSE;
    shknm = ESHK(shkp)->shknam;
    /* skip "+" prefix */
    if (!letter(*shknm))
        ++shknm;
    return (boolean) !strcmp(shknm, "Izchak");
}

#undef VEGETARIAN_CLASS

/*shknam.c*/
