#include <sys/ptrace.h>
#include <sys/syscall.h>
#include "DeviceHandlerALSA.h"
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

DeviceHandlerALSA::DeviceHandlerALSA(const pid_t pid, const string openpath): 
		CommunicatingDeviceHandler(pid, openpath, "alsa", "ALSA")
{
	if(openpath == "/dev/snd/controlC0")
		_devType = Control;
	if(openpath == "/dev/aloadC0")
		_devType = Load;
	if(openpath == "/dev/snd/pcmC0D0p")
		_devType = PCM;

	_card_info.card = 0;
	_card_info.pad = 0;
	strcpy((char*)_card_info.id, "virtcard");
	strcpy((char*)_card_info.driver, "virdcarddrv");
	strcpy((char*)_card_info.name, "virtual soundcard");
	strcpy((char*)_card_info.longname, "virtual soundcard long name");
	strcpy((char*)_card_info.reserved_, "");
	strcpy((char*)_card_info.mixername, "virtual mixer");
	strcpy((char*)_card_info.components, "AC97");

	_pcm_info.device = 0;
	_pcm_info.subdevice = 0;
	_pcm_info.stream = SNDRV_PCM_STREAM_PLAYBACK;
	_pcm_info.card = 0;
	strcpy((char*)_pcm_info.id, "virtual card pcm");
	strcpy((char*)_pcm_info.name, "virtual card pcm");
	strcpy((char*)_pcm_info.subname, "virtual card pcm subname");
	_pcm_info.dev_class = SNDRV_PCM_CLASS_GENERIC;
	_pcm_info.dev_subclass = SNDRV_PCM_SUBCLASS_GENERIC_MIX;
	_pcm_info.subdevices_count = 0;
	_pcm_info.subdevices_avail = 0;
	strcpy((char*)_pcm_info.sync.id, "SYNCID");
	strcpy((char*)_pcm_info.reserved, "");

	_pcm_sync_ptr.flags = 0;

	memset(&_snd_pcm_hw_params, 0, sizeof(_snd_pcm_hw_params));
	memset(&_snd_pcm_status, 0, sizeof(_snd_pcm_status));

	_log.Info("Path:", _openpath);
	_log.Info("Processing device type of ", _devType);
}

string DeviceHandlerALSA::GetFixupScript() const
{
	if(!HasPermissions())
		return GetFixupScriptCore();

	return "";
}

void DeviceHandlerALSA::ExecuteBefore(pid_t pid, const long syscall, user_regs_struct &regs)
{
	_syscallbefore = syscall;

	if(_devType == Control) 
	{
		ExecuteBeforeControl(pid, syscall, regs);
	}
	else if(_devType == PCM)
	{
		ExecuteBeforePCM(pid, syscall, regs);
	}

}

void DeviceHandlerALSA::ExecuteAfter(pid_t pid,const long syscall, user_regs_struct &regs)
{
	_syscallafter = syscall;

	if(_devType == Control) 
	{
		ExecuteAfterControl(pid, syscall, regs);
	}
	else if(_devType == PCM) 
	{
		ExecuteAfterPCM(pid, syscall, regs);
	}
}

void DeviceHandlerALSA::ExecuteBeforeControl(pid_t pid, const long syscall, user_regs_struct &regs)
{
	if(syscall == SYS_ioctl)
	{
		_log.Debug("Regs before Control ioctl. rdi:", regs.rdi, "rsi:", regs.rsi, "rdx:", regs.rdx);
		_ioctlop = regs.rsi;
		_ioctladdr = regs.rdx;

		if(_ioctlop == SNDRV_CTL_IOCTL_CARD_INFO)
		{
			_log.Info("-= Before Control Device ioctl - SNDRV_CTL_IOCTL_CARD_INFO =-");
		}
		else if(_ioctlop == SNDRV_CTL_IOCTL_PVERSION)
		{
			_log.Info("-= Before Control Device ioctl - SNDRV_CTL_IOCTL_PVERSION =-");
		}
		else if(_ioctlop == SNDRV_CTL_IOCTL_PCM_PREFER_SUBDEVICE)
		{
			_log.Info("-= Before Control Device ioctl - SNDRV_CTL_IOCTL_PCM_PREFER_SUBDEVICE =-");
		}
		else 
		{
			_log.Error("-= Before Control Device ioctl - unknown ioctl op =-", _ioctlop);
		}
		regs.orig_rax = -1;
		ptrace(PTRACE_SETREGS, pid, NULL, &regs);
	}
	else
	{
		_log.Info("Other Control syscall:", syscall);
	} 
}

void DeviceHandlerALSA::ExecuteAfterControl(pid_t pid, const long syscall, user_regs_struct &regs)
{
	if(_syscallbefore == SYS_ioctl) 
	{
		if(_ioctlop == SNDRV_CTL_IOCTL_CARD_INFO)
		{
			_log.Info("-= After Control Device ioctl - SNDRV_CTL_IOCTL_CARD_INFO =-");
			PokeData(pid, _ioctladdr, &_card_info, sizeof(_card_info));
			regs.rax = 0;
		}
		else if(_ioctlop == SNDRV_CTL_IOCTL_PVERSION)
		{
			_log.Info("-= After Control Device ioctl - SNDRV_CTL_IOCTL_PVERSION =-");
			int ver = SNDRV_CTL_VERSION;
			PokeData(pid, _ioctladdr, &ver, sizeof(ver));
			regs.rax = 0;
		}
		else if(_ioctlop == SNDRV_CTL_IOCTL_PCM_PREFER_SUBDEVICE)
		{
			_log.Info("-= After Control Device ioctl - SNDRV_CTL_IOCTL_PCM_PREFER_SUBDEVICE =-");
			regs.rax = 0;
		}
		ptrace(PTRACE_SETREGS, pid, NULL, &regs);
	}
	else
	{
		_log.Error("Unhandled Control syscall:", syscall);
	}
	_log.Debug("regs. rax:", regs.rax, "rdi:", regs.rdi, "rsi:", regs.rsi, "rdx:", regs.rdx,
		"r10:", regs.r10, "r8: ", regs.r8, "r9:", regs.r9);
}

void DeviceHandlerALSA::ExecuteBeforePCM(pid_t pid, const long syscall, user_regs_struct &regs)
{
	if(syscall == SYS_ioctl)
	{
		_log.Debug("Regs before PCM Ioctl. rdi:", regs.rdi, "rsi:", regs.rsi, "rdx:", regs.rdx);
		_ioctlop = regs.rsi;
		_ioctladdr = regs.rdx;

		if(_ioctlop == SNDRV_PCM_IOCTL_INFO)
		{
			_log.Info("-= Before PCM Device ioctl - SNDRV_PCM_IOCTL_INFO =-");
		}
		else if(_ioctlop == SNDRV_PCM_IOCTL_PVERSION)
		{
			_log.Info("-= Before PCM Device ioctl - SNDRV_PCM_IOCTL_PVERSION =-");
		}
		else if(_ioctlop == SNDRV_PCM_IOCTL_TTSTAMP)
		{
			_log.Info("-= Before PCM Device ioctl - SNDRV_PCM_IOCTL_TTSTAMP =-");
		}
		else if(_ioctlop == SNDRV_PCM_IOCTL_SYNC_PTR)
		{
			_log.Debug("-= Before PCM Device ioctl - SNDRV_PCM_IOCTL_SYNC_PTR =-");
		}
		else if(_ioctlop == SNDRV_PCM_IOCTL_HW_REFINE)
		{
			_log.Info("-= Before PCM Device ioctl - SNDRV_PCM_IOCTL_HW_REFINE =-");
		}
		else if(_ioctlop == SNDRV_PCM_IOCTL_HW_PARAMS)
		{
			_log.Info("-= Before PCM Device ioctl - SNDRV_PCM_IOCTL_HW_PARAMS =-");
		}
		else if(_ioctlop == SNDRV_PCM_IOCTL_SW_PARAMS)
		{
			_log.Info("-= Before PCM Device ioctl - SNDRV_PCM_IOCTL_SW_PARAMS =-");
		}
		else if(_ioctlop == SNDRV_PCM_IOCTL_PREPARE)
		{
			_log.Info("-= Before PCM Device ioctl - SNDRV_PCM_IOCTL_PREPARE =-");
		}
		else if(_ioctlop == SNDRV_PCM_IOCTL_STATUS)
		{
			_log.Debug("-= Before PCM Device ioctl - SNDRV_PCM_IOCTL_STATUS =-");
		}
#ifdef SNDRV_PCM_IOCTL_STATUS_EXT 
		else if(_ioctlop == SNDRV_PCM_IOCTL_STATUS_EXT)
		{
			_log.Debug("-= Before PCM Device ioctl - SNDRV_PCM_IOCTL_STATUS_EXT =-");
		}
#endif
		else if(_ioctlop == SNDRV_PCM_IOCTL_DELAY)
		{
			_log.Debug("-= Before PCM Device ioctl - SNDRV_PCM_IOCTL_DELAY =-");
		}
		else if(_ioctlop == SNDRV_PCM_IOCTL_DROP)
		{
			_log.Info("-= Before PCM Device ioctl - SNDRV_PCM_IOCTL_DROP =-");
		}
		else if(_ioctlop == SNDRV_PCM_IOCTL_HW_FREE)
		{
			_log.Info("-= Before PCM Device ioctl - SNDRV_PCM_IOCTL_HW_FREE =-");
		}
		else if(_ioctlop == SNDRV_PCM_IOCTL_WRITEI_FRAMES)
		{
			_log.Debug("-= Before PCM Device ioctl - SNDRV_PCM_IOCTL_WRITEI_FRAMES =-");
		}
		else if(_ioctlop == SNDRV_PCM_IOCTL_PAUSE)
		{
			_log.Info("-= Before PCM Device ioctl - SNDRV_PCM_IOCTL_PAUSE =-");
		}
		else if(_ioctlop == SNDRV_PCM_IOCTL_RESUME)
		{
			_log.Info("-= Before PCM Device ioctl - SNDRV_PCM_IOCTL_RESUME =-");
		}
		else 
		{
			_log.Error("unknown ioctl op ", _ioctlop);
		}
		regs.orig_rax = -1;
		ptrace(PTRACE_SETREGS, pid, NULL, &regs);
	}
	else if(syscall == SYS_mmap)
	{
		_log.Info("-= Before PCM Device mmap =-");
		_log.Debug("regs. rdi:", regs.rdi, "rsi:", regs.rsi, "rdx:", regs.rdx,
			"r10:", regs.r10, "r8: ", regs.r8, "r9:", regs.r9);
		regs.orig_rax = -1;
		ptrace(PTRACE_SETREGS, pid, NULL, &regs);
	}
	else
	{
		_log.Info("Other PCM syscall:", syscall);
	} 
}

void DeviceHandlerALSA::ExecuteAfterPCM(pid_t pid, const long syscall, user_regs_struct &regs)
{
	if(_syscallbefore == SYS_ioctl) 
	{
		if(_ioctlop == SNDRV_PCM_IOCTL_INFO)
		{
			_log.Info("-= After PCM Device ioctl - SNDRV_PCM_IOCTL_INFO =-");
			PokeData(pid, _ioctladdr, &_pcm_info, sizeof(_pcm_info));

			if(_socketCommunicator.Open("127.0.0.1", GetPort()))
			{
				_log.Info("socket opened");
				regs.rax = 0;
			}
			else 
			{
				_log.Error("socket open failed");
				regs.rax = -1;
			}

		}
		else if(_ioctlop == SNDRV_PCM_IOCTL_PVERSION)
		{
			_log.Info("-= After PCM Device ioctl - SNDRV_PCM_IOCTL_PVERSION =-");
			int ver = SNDRV_PCM_VERSION;
			PokeData(pid,_ioctladdr, &ver, sizeof(ver));
			regs.rax = 0;
		}
		else if(_ioctlop == SNDRV_PCM_IOCTL_TTSTAMP)
		{
			_log.Info("-= After PCM Device ioctl - SNDRV_PCM_IOCTL_TTSTAMP =-");
			int ttstamp = 0;
			PeekData(pid, _ioctladdr, &ttstamp, sizeof(ttstamp));
			regs.rax = 0;
		}
		else if(_ioctlop == SNDRV_PCM_IOCTL_SYNC_PTR)
		{
			_log.Debug("-= After PCM Device ioctl - SNDRV_PCM_IOCTL_SYNC_PTR =-");
			PeekData(pid, _ioctladdr, &_pcm_sync_ptr, sizeof(_pcm_sync_ptr));
			regs.rax = 0;
		}
		else if(_ioctlop == SNDRV_PCM_IOCTL_HW_REFINE)
		{
			_log.Info("-= After PCM Device ioctl - SNDRV_PCM_IOCTL_HW_REFINE =-");
			PeekData(pid, _ioctladdr, &_snd_pcm_hw_params, sizeof(_snd_pcm_hw_params));
			
			regs.rax = 0;
		}
		else if(_ioctlop == SNDRV_PCM_IOCTL_HW_PARAMS)
		{
			_log.Info("-= After PCM Device ioctl - SNDRV_PCM_IOCTL_HW_PARAMS =-");
			PeekData(pid, _ioctladdr, &_snd_pcm_hw_params, sizeof(_snd_pcm_hw_params));
			
			int format = _snd_pcm_hw_params.masks[SNDRV_PCM_HW_PARAM_FORMAT - 
				SNDRV_PCM_HW_PARAM_FIRST_MASK].bits[0];
			int sampleBitsMin =_snd_pcm_hw_params.intervals[SNDRV_PCM_HW_PARAM_SAMPLE_BITS - 
				SNDRV_PCM_HW_PARAM_FIRST_INTERVAL].min;
			int sampleBitsMax = _snd_pcm_hw_params.intervals[SNDRV_PCM_HW_PARAM_SAMPLE_BITS - 
				SNDRV_PCM_HW_PARAM_FIRST_INTERVAL].max;
			int frameBitsMin = _snd_pcm_hw_params.intervals[SNDRV_PCM_HW_PARAM_FRAME_BITS - 
				SNDRV_PCM_HW_PARAM_FIRST_INTERVAL].min;
			int frameBitsMax = _snd_pcm_hw_params.intervals[SNDRV_PCM_HW_PARAM_FRAME_BITS - 
				SNDRV_PCM_HW_PARAM_FIRST_INTERVAL].max;
			int channelsMin = _snd_pcm_hw_params.intervals[SNDRV_PCM_HW_PARAM_CHANNELS - 
				SNDRV_PCM_HW_PARAM_FIRST_INTERVAL].min;
			int channelsMax = _snd_pcm_hw_params.intervals[SNDRV_PCM_HW_PARAM_CHANNELS - 
				SNDRV_PCM_HW_PARAM_FIRST_INTERVAL].max;
			int rateMin = _snd_pcm_hw_params.intervals[SNDRV_PCM_HW_PARAM_RATE - 
				SNDRV_PCM_HW_PARAM_FIRST_INTERVAL].min;
			int rateMax = _snd_pcm_hw_params.intervals[SNDRV_PCM_HW_PARAM_RATE - 
				SNDRV_PCM_HW_PARAM_FIRST_INTERVAL].max;

			int bufferSizeMax = _snd_pcm_hw_params.intervals[SNDRV_PCM_HW_PARAM_BUFFER_SIZE - 
				SNDRV_PCM_HW_PARAM_FIRST_INTERVAL].max;
			int bufferBytesMax = _snd_pcm_hw_params.intervals[SNDRV_PCM_HW_PARAM_BUFFER_BYTES - 
				SNDRV_PCM_HW_PARAM_FIRST_INTERVAL].max;

			if(sampleBitsMin == sampleBitsMax && frameBitsMin == frameBitsMax &&
				channelsMin == channelsMax && rateMin == rateMax)
			{
				SendOpcode("INIT");
				int initData[4];
				initData[0] = rateMin;
				initData[1] = sampleBitsMin;
				initData[2] = channelsMin;
				initData[3] = format;
				_log.Debug("ALSA hw setup. rate:", rateMin, ", samplebits:", sampleBitsMin, 
					", channels:", channelsMin, ", format:", format);
				_log.Debug("ALSA hw setup. bufsize:", bufferSizeMax, ", bufbytes:", bufferBytesMax);
				_socketCommunicator.Send((char *)&initData, 4*4);
			}
			regs.rax = 0;
		}
		else if(_ioctlop == SNDRV_PCM_IOCTL_SW_PARAMS)
		{
			_log.Info("-= After PCM Device ioctl - SNDRV_PCM_IOCTL_SW_PARAMS =-");
			regs.rax = 0;
		}
		else if(_ioctlop == SNDRV_PCM_IOCTL_PREPARE)
		{
			_log.Info("-= After PCM Device ioctl - SNDRV_PCM_IOCTL_PREPARE =-");
			regs.rax = 0;
		}
		else if(_ioctlop == SNDRV_PCM_IOCTL_STATUS)
		{
			_log.Debug("-= After PCM Device ioctl - SNDRV_PCM_IOCTL_STATUS =-");
			long delay = 0;
			SendOpcode("STAT");
			_socketCommunicator.Recv((char *)&delay, 4);
			_log.Debug("delay in frames:", delay);

			int bufferSizeMax = _snd_pcm_hw_params.intervals[SNDRV_PCM_HW_PARAM_BUFFER_SIZE - 
				SNDRV_PCM_HW_PARAM_FIRST_INTERVAL].max;

			if(bufferSizeMax < 10000)
				bufferSizeMax = 10000;

			int avail = bufferSizeMax - delay;

			if(avail < 0)
				avail = 0;
			_snd_pcm_status.avail = avail;
			PokeData(pid,_ioctladdr, &_snd_pcm_status, sizeof(_snd_pcm_status));
			regs.rax = 0;
		}
#ifdef SNDRV_PCM_IOCTL_STATUS_EXT 
		else if(_ioctlop == SNDRV_PCM_IOCTL_STATUS_EXT)
		{
			_log.Debug("-= After PCM Device ioctl - SNDRV_PCM_IOCTL_STATUS_EXT =-");
			long delay = 0;
			SendOpcode("STAT");
			_socketCommunicator.Recv((char *)&delay, 4);
			_log.Debug("delay in frames:", delay);

			int bufferSizeMax = _snd_pcm_hw_params.intervals[SNDRV_PCM_HW_PARAM_BUFFER_SIZE - 
				SNDRV_PCM_HW_PARAM_FIRST_INTERVAL].max;

			if(bufferSizeMax < 10000)
				bufferSizeMax = 10000;

			int avail = bufferSizeMax - delay;

			if(avail < 0)
				avail = 0;
			_snd_pcm_status.avail = avail;
			PokeData(pid, _ioctladdr, &_snd_pcm_status, sizeof(_snd_pcm_status));
			regs.rax = 0;
		}
#endif
		else if(_ioctlop == SNDRV_PCM_IOCTL_DELAY)
		{
			_log.Debug("-= After PCM Device ioctl - SNDRV_PCM_IOCTL_DELAY =-");
			long delay = 0;
			SendOpcode("DELA");
			_socketCommunicator.Recv((char *)&delay, 4);
			_log.Debug("delay in frames:", delay);
			PokeData(pid, _ioctladdr, &delay, sizeof(delay));
			regs.rax = 0;
		}
		else if(_ioctlop == SNDRV_PCM_IOCTL_DROP)
		{
			_log.Info("-= After PCM Device ioctl - SNDRV_PCM_IOCTL_DROP =-");
			SendOpcode("DROP");
			regs.rax = 0;
		}
		else if(_ioctlop == SNDRV_PCM_IOCTL_HW_FREE)
		{
			_log.Info("-= After PCM Device ioctl - SNDRV_PCM_IOCTL_HW_FREE =-");
			SendOpcode("CLOS");
			regs.rax = 0;
		}
		else if(_ioctlop == SNDRV_PCM_IOCTL_WRITEI_FRAMES)
		{
			_log.Debug("-= After PCM Device ioctl - SNDRV_PCM_IOCTL_WRITEI_FRAMES =-");
			snd_xferi xferi;
			PeekData(pid, _ioctladdr, &xferi, sizeof(xferi));
			
			int framebytes = _snd_pcm_hw_params.intervals[SNDRV_PCM_HW_PARAM_FRAME_BITS - 
				SNDRV_PCM_HW_PARAM_FIRST_INTERVAL].max/8;
			
			unsigned int bytes = xferi.frames * framebytes;
			char *buf = (char *)malloc(bytes);
			PeekData(pid, (unsigned long)xferi.buf, buf, bytes);

			SendOpcode("PLAY");
			_socketCommunicator.Send((char *)&bytes, 4);
			_socketCommunicator.Send(buf, bytes);
			free(buf);
			_log.Debug("Bytes:", bytes);

			xferi.result = xferi.frames;
			PokeData(pid, _ioctladdr, &xferi, sizeof(xferi));
			regs.rax = 0;
		}
		else if(_ioctlop == SNDRV_PCM_IOCTL_PAUSE)
		{
			_log.Info("-= After PCM Device ioctl - SNDRV_PCM_IOCTL_PAUSE =-");
			SendOpcode("PAUS");
			_snd_pcm_status.state = SNDRV_PCM_STATE_SUSPENDED;
			regs.rax = 0;
		}
		else if(_ioctlop == SNDRV_PCM_IOCTL_RESUME)
		{
			_log.Info("-= After PCM Device ioctl - SNDRV_PCM_IOCTL_RESUME =-");
			SendOpcode("RESU");
			_snd_pcm_status.state = SNDRV_PCM_STATE_RUNNING;
			regs.rax = 0;
		}
		ptrace(PTRACE_SETREGS, pid, NULL, &regs);
	}
	else if(_syscallbefore == SYS_mmap)
	{
		regs.rax = 0;
		ptrace(PTRACE_SETREGS, pid, NULL, &regs);
	}
	_log.Debug("regs. rax:", regs.rax, "rdi:", regs.rdi, "rsi:", regs.rsi, "rdx:", regs.rdx,
		"r10:", regs.r10, "r8: ", regs.r8, "r9:", regs.r9);
}

