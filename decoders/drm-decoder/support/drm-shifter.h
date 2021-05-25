

#ifndef	__DRM_SHIFTER__
#define	__DRM_SHIFTER__
#include	"basics.h"

class	drmShifter {
public:
			drmShifter		(int32_t);
			~drmShifter	(void);
	std::complex<JAN>	do_shift (std::complex<JAN>, int32_t);
	void		do_shift	(std::complex<JAN> *,
	                                 int32_t, int32_t);
private:
	int32_t		phase;
	int32_t		tableSize;
	std::complex<JAN>	*table;
};
#endif

