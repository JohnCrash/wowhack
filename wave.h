#ifndef _WAVE_H_
#define _WAVE_H_
#include <mmsystem.h>

class CWaveBuffer  
{
public:
	int GetSampleSize() const;
	void SetBuffer(void* pBuffer, DWORD dwNumSamples, int nSize);
	void SetNumSamples(DWORD dwNumSamples, int nSize = sizeof(short));
	void CopyBuffer(void* pBuffer, DWORD dwNumSamples, int nSize = sizeof(short));
	DWORD GetNumSamples() const;
	void* GetBuffer() const;
	CWaveBuffer();
	virtual ~CWaveBuffer();
private:
	int   m_nSampleSize;
	void* m_pBuffer;
	DWORD m_dwNum;
};

class CWave : public CObject  
{
	DECLARE_SERIAL(CWave)
public:
	void SetBuffer(void* pBuffer, DWORD dwNumSamples, bool bCopy = false);
	bool Load(const CString& strFile);
	BOOL Save(const CString& strFile);
	DWORD GetBufferLength() const;
	DWORD GetNumSamples() const;
	void* GetBuffer() const;
	void Save(CFile* f);
	void Load(CFile* f);
	WAVEFORMATEX GetFormat() const;
	void BuildFormat(WORD nChannels, DWORD nFrequency, WORD nBits);
	CWave();
	CWave(const CWave& copy);
	CWave& operator=(const CWave& wave);
	virtual ~CWave();
	virtual void Serialize( CArchive& archive );
private:
	CWaveBuffer m_buffer;
	WAVEFORMATEX m_pcmWaveFormat;
};

class CWaveDevice  
{
public:
	CWaveDevice(const CWaveDevice& copy);
	UINT GetDevice() const;
	bool IsOutputFormat(const CWave& wave);
	bool IsInputFormat(const CWave& wave);
	CWaveDevice(UINT nDevice = WAVE_MAPPER);
	virtual ~CWaveDevice();

private:
	UINT m_nDevice;
};

//////////////////////////////////////////////////////////////////////
void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

//////////////////////////////////////////////////////////////////////
#ifdef WAVE_IN_BUFFER_SIZE
#undef WAVE_IN_BUFFER_SIZE
#endif
#define WAVEIN_BUFFER_SIZE 4096

#define NUMWAVEINHDR 2

//////////////////////////////////////////////////////////////////////
class CWaveIn  
{
	friend void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
public:
	CString GetError() const;
	DWORD GetPosition();
	bool IsRecording();
	CWave MakeWave();

	bool Close();
	bool Continue();
	bool Open();
	bool Pause();
	bool Record(UINT nTaille = 4096);
	bool Stop();

	void SetDevice(const CWaveDevice& aDevice);
	void SetWaveFormat(WAVEFORMATEX tagFormat);

	CWaveIn();
	CWaveIn(WAVEFORMATEX tagFormat, const CWaveDevice& aDevice);
	virtual	~CWaveIn();
private:
	bool AddNewBuffer(WAVEHDR* pWaveHdr);
	bool AddNewHeader(HWAVEIN hwi);
	void FreeListOfBuffer();
	DWORD GetNumSamples();
	void FreeListOfHeader();
	void InitListOfHeader();
	bool IsError(MMRESULT nResult);
	bool ResetRequired(CWaveIn* pWaveIn);
private:
	bool m_bResetRequired;
	HWAVEIN	m_hWaveIn;
	CPtrList m_listOfBuffer;
	UINT m_nError;
	int	m_nIndexWaveHdr;
	UINT m_nBufferSize;
	WAVEHDR	m_tagWaveHdr[NUMWAVEINHDR];
	CWave m_wave;
	CWaveDevice m_waveDevice;
};

class CWaveInterface  
{
public:
	static CString GetWaveInName(UINT nIndex);
	static UINT GetWaveInCount();
	static CString GetWaveOutName(UINT nIndex);
	static UINT GetWaveOutCount();
};

//////////////////////////////////////////////////////////////////////
void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

//////////////////////////////////////////////////////////////////////
#ifdef WAVE_OUT_BUFFER_SIZE
#undef WAVE_OUT_BUFFER_SIZE
#endif
#define WAVEOUT_BUFFER_SIZE 4096

#define NUMWAVEOUTHDR 3
#define INFINITE_LOOP INT_MAX

//////////////////////////////////////////////////////////////////////
class CWaveOut
{
	friend void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
public:
	CString GetError() const;
	DWORD GetPosition();
	bool IsPlaying();
	
	bool Close();
	bool Continue();
	bool FullPlay(int nLoop = -1, DWORD dwStart = -1, DWORD dwEnd = -1);
	bool Open();
	bool Pause();
	bool Play(DWORD dwStart = -1, DWORD dwEnd = -1);
	bool Stop();

	void ModifyWaveOutBufferLength(DWORD dwLength);
	void SetDevice(const CWaveDevice& aDevice);
	void SetWave(const CWave& aWave);

	CWaveOut();
	CWaveOut(const CWave& aWave, const CWaveDevice& aDevice);
	virtual ~CWaveOut();
private:
	bool AddFullHeader(HWAVEOUT hwo, int nLoop);
	bool AddNewHeader(HWAVEOUT hwo);
	DWORD GetBufferLength();
	bool IsError(MMRESULT nResult);
	bool ResetRequired(CWaveOut* pWaveOut);
private:

	DWORD m_dwEndPos;
	DWORD m_dwStartPos;
	DWORD m_dwWaveOutBufferLength;
	HWAVEOUT m_hWaveOut;
	UINT m_nError;
	int m_nIndexWaveHdr;
	WAVEHDR m_tagWaveHdr[NUMWAVEOUTHDR];
	CWave m_wave;
	CWaveDevice m_waveDevice;
};

#endif