func main()
{
	try
	{
		var host = TCP.Listener("127.0.0.1", 8000)
		var client = host.accept()
	}

	recover when(Error.reason)
	{
		TCP.Listener.Init -> recoverInit()
		TCP.Listener.accept -> recoverAccept()
	}
}

/*
try:
	var host = TCP.Listener("127.0.0.1", 8000)
	if(Error.reason) jump recover

	var client = host.accept()
	if(Error.reason) jump recover

recover:
	if(Error.reason == TCP.Listener.Init())
		recoverInit()

	if(Error.reason == TCP.Listener.accept())
		recoverAccept()

	Error.reason = null
*/
