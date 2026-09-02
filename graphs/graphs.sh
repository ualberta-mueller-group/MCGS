rm -f *.dot.png && find . -regex '.*\.dot$' | xargs -I FILE sh -c 'dot -Tpng FILE > FILE.png'

