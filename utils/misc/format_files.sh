SUMMARY_FILE="format_result.txt"

rm "$SUMMARY_FILE"

rmspace() {
    cat $1 | tr -d '[:space:]'
}

UNSAFE_FILES=( )

for SRC_FILE in $@ ; do
    if [[ $(echo "$SRC_FILE" | grep '___transformed' ) ]] ; then
        continue
    fi

    NEW_FILE="$(echo $SRC_FILE | sed -E 's/(\.cpp|\.h)/___transformed\1/g')"

    echo "$SRC_FILE --> $NEW_FILE"  >> "$SUMMARY_FILE"

    clang-format --style="file:clangFormatConfig" "$SRC_FILE" > "$NEW_FILE"

    diff "$SRC_FILE" "$NEW_FILE" >> "$SUMMARY_FILE"

    diff -q <(rmspace "$SRC_FILE") <(rmspace "$NEW_FILE")

    if [[ $? -ne 0 ]] ; then
        UNSAFE_FILES+=( "$NEW_FILE" )
    fi
done




if [[ "${#UNSAFE_FILES[@]}" -ne 0 ]] ; then
    echo "${#UNSAFE_FILES[@]} file(s) seem to meaningfully change something. Check if it was a comment at the end of a namespace"

    echo "Files:"

    for F in "${UNSAFE_FILES[@]}" ; do
        echo "$F"
    done

fi

