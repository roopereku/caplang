#ifndef CAP_VALIDATION_STATE_HH
#define CAP_VALIDATION_STATE_HH

namespace cap
{

class ValidationState
{
public:
	bool inVariable = false;
	bool findMembersInParent = false;
};

};

#endif
