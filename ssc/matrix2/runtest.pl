#!/usr/bin/perl 
$MAX = 5000;
open(RECORD,">seq_time.dat");
for ($N=1000; $N<$MAX; $N+=1000) {
	system("gcc -DN=$N -O3 -o matrix matrix.c");
	$testrun = 2;
	$total = 0;
	for (1..$testrun)
	{
		open(OUT, "matrix|");
		while (<OUT>)
		{
			$total+=$_;
			print $_;
		}
	}
	$average = $total/$testrun;
	print "N=($N) Time = ($average)\n";
	$w = ($N**3)/$average;
	print RECORD "$N $w $average\n";
}
