--------------------------------------------------
file.cap

namespace File

type File.Handle
{
	func write(data = string)
	{
	}

	func read() string
	{
	}
}

enum Error.Context extend
{
	NoPermission,
	NoSpaceLeft,
	NonExistent
}

func File.open(path = string) File.Handle
{
}

func File.create(path = string) File.Handle
{
	/*

	*/
	errorOut(NoPermission)

	/*

	*/
	errorOut(NoSpaceLeft)
}

--------------------------------------------------
main.cap

import file.cap

func main()
{
	try
	{
		var file = File.create("/tmp/logfile")
	}

	recover when(Error.reason)
	{
		File.create
		{
			when(Error.context)
			{
				Error.Context.NoPermission {}
				Error.Context.NoSpaceLeft {}

				else {}
			}	
		}
	}
}

--------------------------------------------------
