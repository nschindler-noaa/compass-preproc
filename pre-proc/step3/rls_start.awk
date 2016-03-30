$1 == "release"\
{	printf("%s %s %s %s\n", $1, $2, $3, startdate)
	next
}
{print}