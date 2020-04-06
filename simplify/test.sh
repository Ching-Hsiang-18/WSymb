TOOL=./swymplify
EXTENSION=pwf

for f in `ls test`; do
    BNAME=`basename $f .$EXTENSION`
    if [ $BNAME != $f ]; then
        IN=test/$f
        OUT=test/$BNAME".out"
        EXPECT=test/$BNAME".expect"

        if [ -e $EXPECT ]; then
            echo Processing $IN
            $TOOL $IN > $OUT
            diff $OUT $EXPECT
        fi
    fi
done

echo "Done"
