

for i in $(seq 1 355); do
    
	val=1.1
	
	zoom=`echo "$val ^ $i" | bc -l`

	echo $i zoom=$zoom

	./test $zoom

	printf -v j "%05d" $i
	cp test.tga frame$j.tga
done


#convert -loop 0 -delay 20 -layers Optimize -fuzz "10%" frame0*.tga zoom.gif
