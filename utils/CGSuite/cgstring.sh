# Usage: ./cgstring.sh '<CGSuite string (via right click -> Copy)>'
# NOTE: Include single quotes around the CGSuite string

# Strips enclosing double quotes if present
# Replaces <NL> and <TAB> with newline and tab
echo "$1" | sed 's/<TAB>/\t/g;s/<NL>/\n/g;s/^"//g;s/"$//g'
