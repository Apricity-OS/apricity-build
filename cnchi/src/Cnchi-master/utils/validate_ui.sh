#/bin/sh -e

which xmllint >/dev/null 2>&1
if [ $? -ne 0 ]; then
  echo "xmllint not installed. Please install it."
  exit 1
fi

for x in `find /usr/share/cnchi/ui/ -name "*.ui"`
do
  echo Validating $x
  xmllint --noout $x || exit 1
done
