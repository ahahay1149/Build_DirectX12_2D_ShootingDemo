#include "SoundManager.h"

SoundContainer::SoundContainer(const WCHAR* fileName)
{
	HRESULT hr;
	hr = DirectX::LoadWAVAudioFromFileEx(fileName, m_waveFile, m_wavData);

	if (FAILED(hr))
	{
		m_wavData.audioBytes = 0;	
	}
}

HRESULT PlayingSoundContainer::playWavData(ComPtr<IXAudio2>& pXAudio2, DirectX::WAVData* wavData)
{
	HRESULT hr = E_FAIL;
	if (wavData->audioBytes > 0)
	{
		releaseWavData();

		hr = pXAudio2->CreateSubmixVoice(&submix, wavData->wfx->nChannels, wavData->wfx->nSamplesPerSec, 0, 0, 0, 0);
		if (FAILED(hr))
		{
			submix = nullptr;
			return hr;
		}

		XAUDIO2_SEND_DESCRIPTOR submixSend = { 0, submix };	
		
		XAUDIO2_VOICE_SENDS SFXSendList = {1, &submixSend};	

		hr = pXAudio2->CreateSourceVoice(&voice, wavData->wfx,
			0, XAUDIO2_DEFAULT_FREQ_RATIO, NULL, &SFXSendList, NULL);

		if (FAILED(hr))
		{
			submix->DestroyVoice();
			submix = nullptr;

			voice = nullptr;
			return hr;
		}

		submix->SetVolume(1.0f);
		buf.AudioBytes = wavData->audioBytes;
		buf.LoopBegin = wavData->loopStart;
		buf.LoopLength = wavData->loopLength;
		buf.pAudioData = wavData->startAudio;
		buf.pContext = 0;
		buf.PlayBegin = 0;
		buf.Flags = XAUDIO2_END_OF_STREAM;
		buf.PlayLength = 0;

		if (buf.LoopLength == 0)
			buf.LoopCount = 0;
		else
			buf.LoopCount = XAUDIO2_LOOP_INFINITE;

		voice->SubmitSourceBuffer(&buf);

		hr = voice->Start();

		if (SUCCEEDED(hr))
		{
			m_bPlaying = true;
		}
		
	}

	return hr;
}

void PlayingSoundContainer::stop()
{
	if (m_bPlaying && voice)
	{
		m_bPlaying = false;
		voice->Stop();
	}
}

void PlayingSoundContainer::releaseWavData()
{
	if (voice)
	{
		submix->DestroyVoice();
		voice->DestroyVoice();

		submix = nullptr;
		voice = nullptr;

		m_bPlaying = false;
	}
}

void PlayingSoundContainer::update()
{
	XAUDIO2_VOICE_STATE state;

	if (voice && m_bPlaying)
	{
		voice->GetState(&state);
		if (state.BuffersQueued < 1)
		{
			m_bPlaying = false;
		}
	}
}

PlayingSoundContainer::~PlayingSoundContainer()
{
	stop();
	releaseWavData();
}


SoundManager::SoundManager()
{
	m_seCount = 0;		 
	m_playIndex = 0;	 
	m_pMasteringVoice = nullptr;	

	for (int i = 0; i < MAX_SOUNDS; i++)
	{
		m_playingContainers.push_back(make_unique<PlayingSoundContainer>());
	}
}

SoundManager* SoundManager::getInstance()
{
	static unique_ptr<SoundManager> instance = nullptr;

	if (instance.get() == nullptr)
	{
		instance.reset(new SoundManager());
	}

	return instance.get();
}

SoundManager::~SoundManager()
{
	m_soundContainers.clear();
	m_playingContainers.clear();
	
	if (m_pMasteringVoice != nullptr)
	{
		m_pMasteringVoice->DestroyVoice();
		m_pMasteringVoice = nullptr;
	}
	m_pXaudio2.Reset();					
}

HRESULT SoundManager::initSoundManager()
{
	HRESULT hr = E_FAIL;

	hr = XAudio2Create(m_pXaudio2.GetAddressOf(), 0);
	if (FAILED(hr))
	{
		m_pXaudio2 = nullptr;
		return hr;
	}

	hr = m_pXaudio2->CreateMasteringVoice(&m_pMasteringVoice);
	if (FAILED(hr))
	{
		m_pMasteringVoice = nullptr;
		m_pXaudio2 = nullptr;
		return hr;
	}

	m_pMasteringVoice->SetVolume(1.0f);	
	return hr;
}

void SoundManager::destructSoundManager()
{
}

bool SoundManager::loadSoundFile(const WCHAR* fileName, int& soundId)
{
	unique_ptr<SoundContainer> seCon(new SoundContainer(fileName));  

	if (seCon->getWavData()->audioBytes == 0)
	{
		soundId = -1;	
		return false;
	}

	m_soundContainers.push_back(std::move(seCon));
	soundId = m_seCount;
	m_seCount++;
	return true;
}

bool SoundManager::deleteSoundFile(int soundId)
{
	if (soundId < m_seCount)
	{
		auto soundCon = m_soundContainers.begin() + soundId;
		m_soundContainers.erase(soundCon);
		return true;
	}
	return false;
}

int SoundManager::play(UINT soundId)
{
	int index = -1;

	for (int i = m_playIndex; i < MAX_SOUNDS; i++)
	{
		if (m_playingContainers[i]->isFinished())
		{
			index = i;
			break;
		}
	}

	if (index < 0)
	{
		for (int i = 0; i < m_playIndex; i++)
		{
			if (m_playingContainers[i]->isFinished())
			{
				index = i;
				break;
			}
		}
	}

	if (index >= 0)
	{
		if (SUCCEEDED(m_playingContainers[index]->playWavData(m_pXaudio2, m_soundContainers[soundId]->getWavData())))
		{
			m_playIndex = (index + 1) % MAX_SOUNDS;
		}
	}
	return index;
}

void SoundManager::stop(UINT playingId)
{
	if (playingId > -1 && playingId < MAX_SOUNDS)
	{
		m_playingContainers[playingId]->stop();
	}
}

void SoundManager::audioUpdate()
{
	for (int i = 0; i < MAX_SOUNDS; i++)
	{
		m_playingContainers[i]->update();
	}
}

