#ifndef ENVSETTING_H_
#define ENVSETTING_H_

/// Helper class to get environment parameters

class EnvSetting {
public:
	/*---------------------------------------------------------------------------
	FUNCTION: Get
	PURPOSE:
		get an environment parameter value
	PARAMETERS:
		name:		environment parameter name
		defValue:	default value if the environment parameter absent
	RETURN VALUE:
		environment parameter value
	EXCEPTION:
	EXAMPLE CALL:
	REMARKS:
	---------------------------------------------------------------------------*/
	static const char *Get(const char *name, const char *defValue);

private:
	EnvSetting();
};

#endif /* ENVSETTING_H_ */
