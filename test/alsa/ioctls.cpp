#include <iostream>
#include <sys/ioctl.h>
#include <sound/asound.h>

using namespace std;

int main()
{
	cout << "CTL" << endl;
	cout << SNDRV_CTL_IOCTL_PVERSION << endl;
	cout << SNDRV_CTL_IOCTL_CARD_INFO<< endl; 
	cout << SNDRV_CTL_IOCTL_ELEM_LIST<< endl; 
	cout << SNDRV_CTL_IOCTL_ELEM_INFO<< endl; 
	cout << SNDRV_CTL_IOCTL_ELEM_READ<< endl; 
	cout << SNDRV_CTL_IOCTL_ELEM_WRITE<< endl; 
	cout << SNDRV_CTL_IOCTL_ELEM_LOCK<< endl; 
	cout << SNDRV_CTL_IOCTL_ELEM_UNLOCK<< endl; 
	cout << SNDRV_CTL_IOCTL_SUBSCRIBE_EVENTS << endl; 
	cout << SNDRV_CTL_IOCTL_ELEM_ADD<< endl; 
	cout << SNDRV_CTL_IOCTL_ELEM_REPLACE<< endl; 
	cout << SNDRV_CTL_IOCTL_ELEM_REMOVE<< endl; 
	cout << SNDRV_CTL_IOCTL_TLV_READ<< endl; 
	cout << SNDRV_CTL_IOCTL_TLV_WRITE<< endl; 
	cout << SNDRV_CTL_IOCTL_TLV_COMMAND<< endl; 
	cout << SNDRV_CTL_IOCTL_HWDEP_NEXT_DEVICE << endl; 
	cout << SNDRV_CTL_IOCTL_HWDEP_INFO<< endl; 
	cout << SNDRV_CTL_IOCTL_PCM_NEXT_DEVICE<< endl; 
	cout << SNDRV_CTL_IOCTL_PCM_INFO<< endl; 
	cout << SNDRV_CTL_IOCTL_PCM_PREFER_SUBDEVICE << endl; 
	cout << SNDRV_CTL_IOCTL_RAWMIDI_NEXT_DEVICE << endl; 
	cout << SNDRV_CTL_IOCTL_RAWMIDI_INFO<< endl; 
	cout << SNDRV_CTL_IOCTL_RAWMIDI_PREFER_SUBDEVICE << endl; 
	cout << SNDRV_CTL_IOCTL_POWER<< endl; 
	cout << SNDRV_CTL_IOCTL_POWER_STATE<< endl; 

	cout << "PCM" << endl;
	cout << SNDRV_PCM_IOCTL_PVERSION<< endl;
	cout << SNDRV_PCM_IOCTL_INFO<< endl;
	cout << SNDRV_PCM_IOCTL_TSTAMP<< endl;
	cout << SNDRV_PCM_IOCTL_TTSTAMP<< endl;
	cout << SNDRV_PCM_IOCTL_HW_REFINE<< endl;
	cout << SNDRV_PCM_IOCTL_HW_PARAMS<< endl;
	cout << SNDRV_PCM_IOCTL_HW_FREE<< endl;
	cout << SNDRV_PCM_IOCTL_SW_PARAMS<< endl;
	cout << SNDRV_PCM_IOCTL_STATUS<< endl;
	cout << SNDRV_PCM_IOCTL_DELAY<< endl;
	cout << SNDRV_PCM_IOCTL_HWSYNC<< endl;
	cout << SNDRV_PCM_IOCTL_SYNC_PTR<< endl;
	cout << SNDRV_PCM_IOCTL_CHANNEL_INFO<< endl;
	cout << SNDRV_PCM_IOCTL_PREPARE<< endl;
	cout << SNDRV_PCM_IOCTL_RESET<< endl;
	cout << SNDRV_PCM_IOCTL_START<< endl;
	cout << SNDRV_PCM_IOCTL_DROP<< endl;
	cout << SNDRV_PCM_IOCTL_DRAIN<< endl;
	cout << SNDRV_PCM_IOCTL_PAUSE<< endl;
	cout << SNDRV_PCM_IOCTL_REWIND<< endl;
	cout << SNDRV_PCM_IOCTL_RESUME<< endl;
	cout << SNDRV_PCM_IOCTL_XRUN<< endl;
	cout << SNDRV_PCM_IOCTL_FORWARD<< endl;
	cout << SNDRV_PCM_IOCTL_WRITEI_FRAMES<< endl;
	cout << SNDRV_PCM_IOCTL_READI_FRAMES<< endl;
	cout << SNDRV_PCM_IOCTL_WRITEN_FRAMES<< endl;
	cout << SNDRV_PCM_IOCTL_READN_FRAMES<< endl;
	cout << SNDRV_PCM_IOCTL_LINK<< endl;
	cout << SNDRV_PCM_IOCTL_UNLINK<< endl;

	return 0;
}

