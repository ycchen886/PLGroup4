========
README 
========

1. go to the ./preprocess/ directory
2. ./preprocess.sh ../data/1.txt ../data/1.out
3. go to the ./ngram/ directory
4. ./text2ngram ../data/1.out ../data/1
5. ./ngram [unigram_file] [bigram_file] [backward_bigram_file] [trigram_file] [backward_trigram_file] -num [number] -type [WORD/SENTENCE/PARAGRAPH] [-length [SHORT/LONG]] [-keywords [list of keywords]]

========
EXAMPLES
========

./ngram ../data/1_uni.out ../data/1_bi.out ../data/1_bbi.out ../data/1_tri.out ../data/1_btri.out -type SENTENCE -keywords home king land

--------

./ngram ../data/1_uni.out ../data/1_bi.out ../data/1_bbi.out ../data/1_tri.out ../data/1_btri.out -num 10 -type WORD 

./ngram ../data/1_uni.out ../data/1_bi.out ../data/1_bbi.out ../data/1_tri.out ../data/1_btri.out -num 50 -type WORD -length SHORT

./ngram ../data/1_uni.out ../data/1_bi.out ../data/1_bbi.out ../data/1_tri.out ../data/1_btri.out -num 50 -type WORD -length LONG

--------

./ngram ../data/1_uni.out ../data/1_bi.out ../data/1_bbi.out ../data/1_tri.out ../data/1_btri.out -num 3 -type SENTENCE

./ngram ../data/1_uni.out ../data/1_bi.out ../data/1_bbi.out ../data/1_tri.out ../data/1_btri.out -num 10 -type SENTENCE -length SHORT

Her son . 
Lord George . 
A rival legend places his clan . 
Such Protestant superstition . 
James’s parting legacy to them dear . 
But , from Killiecrankie . 
We light on virgin soil . 
For now . 
On either hand . 
The secret of nature . 

./ngram ../data/1_uni.out ../data/1_bi.out ../data/1_bbi.out ../data/1_tri.out ../data/1_btri.out -num 10 -type SENTENCE -keywords king -length SHORT

he king could not save Robin’s neck .
The king David’s crown .
James III. , but for king .
It has been conjectured that this king .
Yet one loves to the king .
I am tempted to serve a king .
The king had to the victim .
Stewarts or Stuarts ( boy king of Nineveh .
Above Fortingall , lies the king .
The king retreated through Badenoch .

./ngram ../data/1_uni.out ../data/1_bi.out ../data/1_bbi.out ../data/1_tri.out ../data/1_btri.out -num 10 -type SENTENCE -keywords he land -length SHORT

which he made of land and adventurous . 
But he carried their land of Shaw . 
Strathallan , he called for this land westward , ” 
Thou , he lived , this land on , ” 
May , he conceived of land on it . 
Forteviot , he in this land of Glasgow . 
’ ; he was , are Scottish land of imagination . 
Through one , he not of land on ! 
Buchanan , this land of Dunblane , he “ tail . 
King of land from Loch , he felt . 

--------

./ngram ../data/1_uni.out ../data/1_bi.out ../data/1_bbi.out ../data/1_tri.out ../data/1_btri.out -num 3 -type PARAGRAPH -keywords war -length SHORT

In Quentin Durward the growth of a flourish of whom I have told how it is pronounced Scoon , as Rob had moved their main seat on hand to every cocked hat that Nature has given much attention by no means the chief heard of as wanting hereabouts , one knows , is wearing away from neighbours , who respected his fellow-soldiers apparently private war ; a dozen soldiers . My mither never saw my own youth , a visit to his in Glenlyon deserves better than their face of woods and fields , baked for centuries . So well for troublesome charge , for this , we may be true , the Ruthven come up , the process there hindered perhaps the oldest tree in a summer on the top be the time when house . The tenant of a chapter on the Breadalbane Campbells had claws as hairy and shaggy glens , their feelings boiled over . On the west a suit the country . Other accounts represent her as he may come in the upsetting time ; but the Romans garrisoned rather than ever , he cut out more dour and sour , not to Scott the Strathyre wedding interrupted by a mausoleum for an atrocity would hardly have been at an early in the fight . 

Most of the Drummonds from Hungary or the leader of this exciting experience as went to a mission in India prevented Sir Walter Scott’s possession of her only twopence . Time was when she seems to a head for its backsliding in mountains of Bruce , and his midnight lair by Fitzjames’s hounds ; on the Highland fairies were transplanted from Tirol to perform all but Rob did visit any but all three centuries ’ standing at changes of deficient and that tunnel through Moncrieff Hill , he in turn of a pen ! Farewell to guess this priest was called the Nairne of this tourist-haunted vicinity . When one had , too , in warrior’s fight That private war could act as precentor , and proposed to look up at the heels of such and such itinerant dealers , who sought for purity of saving truth ; and appears to have shared Balquhidder on the Wicks of local colour in letting his spear like any loathly dragon . We shall see her husband being weathered down and photographing on memory characteristically charming landscapes , could boast of being “ the Glengyle chieftain interfered in her and such-like . The fairies were ignorant Parisians who , as well as a brother of him in their present , who to an island . It sank into misery and extinction , having a “ stickit minister borrowed a Euclid , so now become a chemical one , as a Berwickshire laird . 

Rob Roy rests at Newburgh , surrounded on his way for the colour derived from green cup filled to the Norman conquerors of the same experience as in war could praise the soil , But while those pundits of the Argyllshire gates of the Dochart suggest an incident in an adjacent chapel . An old sycamore trees , the enthusiastic historian shows a distance of some difficulty . In the wings of the Calton Hill , where in being slain by a Campbell bride ; but its foot , and I speak to the Stewarts , lit only trying the river near the most flourishing congregations at peace with the pranks he played with great complacency of Lorne . The exiled Duke of Albany . Their victim of sweeter strain of quasi-royal blood , slaughters , and Traquair House , who in which Lowland manufacture . 
