type A
{
	var y = 3
	var x = u64
}

type T
{
	var y = 2
	var z = A.y * y
}

func main()
{
	var i = A
	var x = T.y + A
}
