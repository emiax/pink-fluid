#! /bin/bash

TIMEFORMAT="%R"
TIME=$(
	{
		time $( 
			for i in {1..10}
			do
				$2
			done
		) > stdout 2> stderr
	} 2>&1
	) 
AVERAGE=$(echo "$TIME/10.0" | bc -l)
echo $AVERAGE