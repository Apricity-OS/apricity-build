iso_name=apricity_os
edition=gnome
remove_prev=true
verbose="-v"

while getopts 'N:V:L:A:D:R:E:w:o:vh' arg; do
    case "${arg}" in
        N) iso_name="${OPTARG}" ;;
        R) remove_prev="${OPTARG}" ;;
        E) edition="${OPTARG}" ;;
        v) verbose="-v" ;;
        *)
           echo "Invalid argument '${arg}'" ;;
    esac
done

rm out/*
git pull origin master
./build.sh ${verbose} -E ${edition} -R ${remove_prev} -N ${iso_name}
scp out/* apricity@apricityos.com:public_html/freezedry-build/${iso_name}.iso
rm out/*
