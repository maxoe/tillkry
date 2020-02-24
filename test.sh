#!/bin/bash
key='key.bin'
FILES=tests/*
echo $FILES
for f in $FILES
do
	msg=$(cat $f)
	input=$(cat $key)${msg}
	test1=$(openssl aes-128-ecb -nopad -in $f -K `cat $key | xxd -p` | xxd -p)
	start=$(date +"%s")
	test2=$(echo $input | ./aes | xxd -p)
	end=$(date +"%s")

	if [ "$test1" = "$test2" ]
	then
		echo "[+] Passed: $f (time elapsed $(expr $end - $start)s)"
	else
		echo "[-] Failed: $f (time elapsed $(expr $end - $start)s)"
	fi
done