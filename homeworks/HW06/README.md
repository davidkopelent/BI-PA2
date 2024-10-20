# Podposloupnosti

Úkolem je realizovat šablonu třídy, která dokáže vyhledávat podřetězce, které se v zadaném řetězci vyskytují opakovaně (tento problém může být zajímavý pro kompresi dat nebo pro hledání plagiátů). Aby byl implementovaný algoritmus dostatečně obecný, bude implementovaný v podobě generické třídy a bude pracovat s obecnými posloupnostmi prvků.

Předpokládáme obecnou posloupnost dat. Tím může být například řetězec (posloupnost znaků), pole celých čísel, seznam párů, ... Třída bude pracovat s obecnými posloupnostmi, následující text ale bude pro názornost používat řetězcovou terminologii (řetězec, podřetězec). Nepřerušený výřez zadaného vstupního řetězce (zkráceně podřetězec) se v původním řetězci může vyskytovat na více různých místech. Například v řetězci abcXabcYabc se podřetězec abc vyskytuje 3x, v řetězci ababab se podřetězec abab vyskytuje 2x (od pozice 0 a od pozice 2).

Prvním řešeným problémem bude nalezení délky nejdelšího podřetězce, který se v zadaném řetězci vyskytuje alespoň N-krát. Například pro řetězec ababab a N=1 bude nalezená délka 6 (celý řetězec), pro N=2 bude nalezená délka 4 (řetězec abab od pozice 0 a 2), pro N=3 bude nalezená délka 2 (řetězec ab od pozic 0, 2 a 4) a pro větší N bude nalezená délka 0.

Druhým řešeným problémem bude nalezení kombinací pozic výskytu nejdelšího podřetězce, který se v zadaném řetězci vyskytuje alespoň N-krát. Výsledkem bude vektor N-tic, kde hodnotou je pozice výskytu podřetězce v původním řetězci. Například pro řetězec abcXabcYabcZdefXdef a N=2 máme dva různé nejdelší podřetězce, které se v původním řetězci vyskytují alespoň 2x: řetězec abc na pozicích 0, 4 a 8 a řetězec def na pozicích 12 a 16. Výsledkem jsou všechny kombinace výskytů, tedy pro podřetězec abc máme 3 kombinace (kombinační číslo 3 nad 2) a pro podřetězec def máme jednu kombinaci výskytu (kombinační číslo 2 nad 2):

[0, 4]
[0, 8]
[4, 8]
[12, 16]
Povšimněte si, že ve výsledku není dvojice [0, 12] (jsou to různé řetězce), dále si všimněte, že hodnoty indexů v každé dvojici mají rostoucí velikost.
Rozhraní třídy CSelfMatch
konstruktor ( initializer_list )
inicializuje vyhledávač, zpracovávaná posloupnost prvků je předána jako std::initializer_list.
konstruktor ( container )
inicializuje vyhledávač, zpracovávaná posloupnost prvků bude vykopírována ze zadaného kontejneru. Kontejnerem může být libovolný STL kontejner, který poskytuje dopředný iterátor.
konstruktor ( begin, end )
inicializuje vyhledávač, zpracovávaná posloupnost prvků bude vykopírována z rozmezí zadané dvojice iterátorů. Předané iterátory splňují nejméně vlastnosti dopředného iterátoru.
push_back ( ... )
přidává prvky do zpracovávané posloupnosti. Parametrem může být libovolný počet prvků, které mají být přidány na konec existující posloupnosti. Metoda může být volána opakovaně. Implementace této metody není povinná, viz níže.
size_t sequenceLen ( n )
metoda vyhledá v uložené posloupnosti nejdelší (spojitou) podposloupnost, která se vyskytuje alespoň n-krát. Návratovou hodnotou je délka této nalezené nejdelší podposloupnosti. Pokud neexistuje žádná podposloupnost, která by se vyskytla alespoň n-krát, bude výsledkem hodnota 0. Pro n=0 vyhoďte výjimku std::invalid_argument.
std::vector<std::array<size_t, N> >findSequences<N> ()
metoda vyhledá v uložené posloupnosti nejdelší (spojitou) podposloupnost, která se v uložené posloupnosti nachází alespoň N-krát. Pro nalezené nejdelší podposloupnosti určí pozice výskytu v původním řetězci a vrátí vektor s N-ticemi všech kombinací výskytů. Návratovou hodnotou je std::vector<std::array<size_t, N> >, tedy N-tice jsou ukládány jako pole fixní délky N. Aby šlo parametr N použít v deklaraci typu, je hodnota N předána jako generický parametr metody findSequences. Pro N=0 vyhoďte výjimku std::invalid_argument.
T_
generický parametr třídy určuje typ prvku ukládané/zpracovávané posloupnosti. Prvek má garantované pouze minimalistické rozhraní: kopírování, přesouvání, destrukci a porovnání na shodu/různost operátory == a !=. Ostatní operace nemusí existovat. Ukázka minimalistického prvku je třída CDummy v přiloženém souboru.
Odevzdávejte zdrojový kód s implementací šablony třídy CSelfMatch. Za základ implementace použijte přiložený zdrojový kód. Pokud v kódu ponecháte bloky podmíněného překladu, lze takový zdrojový kód lokálně testovat a zároveň jej odevzdávat Progtestu.

Hodnocení je rozděleno na povinnou, nepovinnou a bonusovou část:

V povinné části se testují pouze krátké posloupnosti a zadávaný počet výskytů N je nejvýše roven 3. Pro zvládnutí těchto testů postačuje rozumně efektivní implementace naivního algoritmu.
V nepovinné části se testují pouze krátké posloupnosti, zadávaný počet výskytů N ale není omezen. Pro zvládnutí stále postačuje rozumně efektivní implementace naivního algoritmu.
První bonusový test požaduje, aby se správně dedukoval generický parametr třídy CSelfMatch. Dále se používá metoda push_back, kterou se přidává různý počet prvků. Zvládnutí testu vyžaduje detailnější studium záludností šablon v C++ (deduction guides, parametr packs, fold expressions). Pokud Vaše implementace tato vylepšení nezvládá, ponechte vypnutou direktivu TEST_EXTRA_INTERFACE (výchozí stav). Direktivu zapněte pouze pokud vylepšení skutečně implementujete. Zapnutá direktiva bez implementovaných vylepšení povede k chybě při překladu.
Druhý a třetí bonusový test zadává delší posloupnosti (tisíce/desítky tisíc prvků) a velké hodnoty N. Naivní algoritmus takový problém nedokáže vyřešit v rozumném čase, je potřeba použít algoritmus efektivnější. V testech rychlosti jsou většinou zadávané posloupnosti celých čísel a je volaná zejména metoda sequenceLen.
