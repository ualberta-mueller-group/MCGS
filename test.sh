echo -ne "\e[3J" && clear


sleep 0.7
echo "NONE"
./MCGS --no-subgame-split --no-simplify-basic-cgt-games "[clobber_1xn] XOXOXOXOXOXOXOXOXOXOXOXO {B}"
echo ""

sleep 0.7
echo "SPLIT"
./MCGS --no-simplify-basic-cgt-games "[clobber_1xn] XOXOXOXOXOXOXOXOXOXOXOXO {B}"
echo ""

sleep 0.7
echo "SIMPLIFY"
./MCGS --no-subgame-split "[clobber_1xn] XOXOXOXOXOXOXOXOXOXOXOXO {B}"
echo ""

sleep 0.7
echo "ALL"
./MCGS "[clobber_1xn] XOXOXOXOXOXOXOXOXOXOXOXO {B}"
