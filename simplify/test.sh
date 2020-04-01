TOOL=./simplify

for f in `ls test`; do
    BNAME=`basename $f .in`
    if [ $BNAME != $f ]; then
        IN=test/$f
        OUT=test/$BNAME".out"
        EXPECT=test/$BNAME".expect"

        $TOOL $IN > $OUT
        diff $OUT $EXPECT
        if [ $? -eq 1 ]; then
            echo Diff non-empty for \"$IN\"
        fi
    fi
done

echo "All done"
