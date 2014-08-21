exe=$1
corefile=$2

if [ "$exe" = "" ]
then
    echo "must specify a core"
    exit 1
fi

if [ "$corefile" = "" ]
then
    echo "must specify a core"
    exit 1
fi

if [ ! -e $corefile ] 
then
    echo "core file non-existant"
    exit 1
fi

if (file $corefile | grep ELF > /dev/null)
then
    true
else
    echo -n "warning: doesn't look like a corefile: "
    file $corefile
fi

outstem=`basename $corefile`

gdb -q -q $exe $corefile << _ > stacks.$outstem.txt
thread apply all bt full
_

(gdb -q -q $exe $corefile << _  | awk '
BEGIN   { running=0; }
/Shared library/  { running=0; }
//      {
            if (running)
                print $NF;
        }
/From.*To/ { running=1; }'
i sh
_
) | sort | uniq | xargs tar hfc - stacks.$outstem.txt | gzip > info-$outstem.tgz
