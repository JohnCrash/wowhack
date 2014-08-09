#include "stdafx.h"
#include "wave.h"

//////////////////////////////////////////////////////////////////////
// CWaveBuffer
//////////////////////////////////////////////////////////////////////
CWaveBuffer::CWaveBuffer() : m_dwNum(0), m_pBuffer(NULL), m_nSampleSize(0)
{
}

CWaveBuffer::~CWaveBuffer()
{
	m_dwNum = 0L;
	delete[] m_pBuffer;
	m_pBuffer = NULL;
}

void* CWaveBuffer::GetBuffer() const
{
	return m_pBuffer;
}

DWORD CWaveBuffer::GetNumSamples() const
{
	return m_dwNum;
}

void CWaveBuffer::CopyBuffer(void* pBuffer, DWORD dwNumSamples, int nSize)
{
	ASSERT(dwNumSamples >= 0);
	ASSERT(nSize);

	if (!m_pBuffer)
		SetNumSamples(dwNumSamples, nSize);

	if (__min(m_dwNum, dwNumSamples) * nSize > 0) {
		ZeroMemory(m_pBuffer, m_dwNum * m_nSampleSize);
		CopyMemory(m_pBuffer, pBuffer, __min(m_dwNum, dwNumSamples) * nSize);
	}
}

void CWaveBuffer::SetNumSamples(DWORD dwNumSamples, int nSize)
{
	ASSERT(dwNumSamples >= 0);
	ASSERT(nSize > 0);

	void* pBuffer = NULL;

	pBuffer = new char[nSize * dwNumSamples];
	SetBuffer(pBuffer, dwNumSamples, nSize);
}

void CWaveBuffer::SetBuffer(void *pBuffer, DWORD dwNumSamples, int nSize)
{
	ASSERT(dwNumSamples >= 0);
	ASSERT(nSize);

	delete[] m_pBuffer;
	m_pBuffer = pBuffer;
	m_dwNum = dwNumSamples;
	m_nSampleSize = nSize;
}

int CWaveBuffer::GetSampleSize() const
{
	return m_nSampleSize;
}

/////////////////////////////////////////////////////////////////////////////////////
//   CWave
/////////////////////////////////////////////////////////////////////////////////////
IMPLEMENT_SERIAL(CWave, CObject, 1)

CWave::CWave()
{
	ZeroMemory((void*)&m_pcmWaveFormat, sizeof(m_pcmWaveFormat));
	m_pcmWaveFormat.wFormatTag = 1;
}

CWave::CWave(const CWave &copy)
{
	m_pcmWaveFormat = copy.GetFormat();
	m_buffer.SetNumSamples( copy.GetNumSamples(), copy.GetFormat().nBlockAlign ) ;
	m_buffer.CopyBuffer( copy.GetBuffer(), copy.GetNumSamples(), copy.GetFormat().nBlockAlign );
}

CWave& CWave::operator =(const CWave& wave)
{
	if (&wave != this) {
		m_pcmWaveFormat = wave.GetFormat();
		m_buffer.SetNumSamples( wave.GetNumSamples(), wave.GetFormat().nBlockAlign );
		m_buffer.CopyBuffer( wave.GetBuffer(), wave.GetNumSamples(), wave.GetFormat().nBlockAlign );
	}
	return *this;
}

CWave::~CWave()
{
}

void CWave::BuildFormat(WORD nChannels, DWORD nFrequency, WORD nBits)
{
	m_pcmWaveFormat.wFormatTag = WAVE_FORMAT_PCM;
	m_pcmWaveFormat.nChannels = nChannels;
	m_pcmWaveFormat.nSamplesPerSec = nFrequency;
	m_pcmWaveFormat.nAvgBytesPerSec = nFrequency * nChannels * nBits / 8;
	m_pcmWaveFormat.nBlockAlign = nChannels * nBits / 8;
	m_pcmWaveFormat.wBitsPerSample = nBits;
	m_buffer.SetNumSamples(0L, m_pcmWaveFormat.nBlockAlign);
}	

WAVEFORMATEX CWave::GetFormat() const
{
	return m_pcmWaveFormat;
}

void CWave::Serialize( CArchive& archive )
{
	CFile* f = archive.GetFile();
	if (archive.IsLoading())
		Load(f);
	else
		Save(f);
}

bool CWave::Load(const CString &strFile)
{
	CFile f;
	bool b = false;
	if( f.Open(strFile, CFile::modeRead) ){
		Load(&f);
		f.Close();
		b = true;
	}
	return b;
}

void CWave::Load(CFile *f)
{
	char szTmp[10];
	WAVEFORMATEX pcmWaveFormat;
	ZeroMemory(szTmp, 10 * sizeof(char));
	f->Read(szTmp, 4 * sizeof(char)) ;
	if (strncmp(szTmp, _T("RIFF"), 4) != 0) 
		::AfxThrowFileException(CFileException::invalidFile, -1, f->GetFileName());
	DWORD dwFileSize/* = m_buffer.GetNumSamples() * m_pcmWaveFormat.nBlockAlign + 36*/ ;
	f->Read(&dwFileSize, sizeof(dwFileSize)) ;
	ZeroMemory(szTmp, 10 * sizeof(char));
	f->Read(szTmp, 8 * sizeof(char)) ;
	if (strncmp(szTmp, _T("WAVEfmt "), 8) != 0) 
		::AfxThrowFileException(CFileException::invalidFile, -1, f->GetFileName());
	DWORD dwFmtSize /*= 16L*/;
	f->Read(&dwFmtSize, sizeof(dwFmtSize)) ;
	f->Read(&pcmWaveFormat.wFormatTag, sizeof(pcmWaveFormat.wFormatTag)) ;
	f->Read(&pcmWaveFormat.nChannels, sizeof(pcmWaveFormat.nChannels)) ;
	f->Read(&pcmWaveFormat.nSamplesPerSec, sizeof(pcmWaveFormat.nSamplesPerSec)) ;
	f->Read(&pcmWaveFormat.nAvgBytesPerSec, sizeof(pcmWaveFormat.nAvgBytesPerSec)) ;
	f->Read(&pcmWaveFormat.nBlockAlign, sizeof(pcmWaveFormat.nBlockAlign)) ;
	f->Read(&pcmWaveFormat.wBitsPerSample, sizeof(pcmWaveFormat.wBitsPerSample)) ;
	ZeroMemory(szTmp, 10 * sizeof(char));
	f->Read(szTmp, 4 * sizeof(char)) ;
	if (strncmp(szTmp, _T("data"), 4) != 0) 
		::AfxThrowFileException(CFileException::invalidFile, -1, f->GetFileName());
	m_pcmWaveFormat = pcmWaveFormat;
	DWORD dwNum;
	f->Read(&dwNum, sizeof(dwNum)) ;
	m_buffer.SetNumSamples(dwNum / pcmWaveFormat.nBlockAlign, pcmWaveFormat.nBlockAlign);
	f->Read(m_buffer.GetBuffer(), dwNum) ;
}

BOOL CWave::Save(const CString &strFile)
{
	TRY{
		CFile f;
		if( f.Open(strFile, CFile::modeCreate | CFile::modeWrite) ){
			Save(&f);
			f.Close();
		}else return FALSE;
	}CATCH( CFileException,pEx ){
		return FALSE;
	}END_CATCH
	return TRUE;
}

void CWave::Save(CFile *f)
{
	ASSERT( m_buffer.GetNumSamples() > 0 );

	f->Write("RIFF", 4) ;
	DWORD dwFileSize = m_buffer.GetNumSamples() * m_pcmWaveFormat.nBlockAlign + 36 ;
	f->Write(&dwFileSize, sizeof(dwFileSize)) ;
	f->Write("WAVEfmt ", 8) ;
	DWORD dwFmtSize = 16L;
	f->Write(&dwFmtSize, sizeof(dwFmtSize)) ;
	f->Write(&m_pcmWaveFormat.wFormatTag, sizeof(m_pcmWaveFormat.wFormatTag)) ;
	f->Write(&m_pcmWaveFormat.nChannels, sizeof(m_pcmWaveFormat.nChannels)) ;
	f->Write(&m_pcmWaveFormat.nSamplesPerSec, sizeof(m_pcmWaveFormat.nSamplesPerSec)) ;
	f->Write(&m_pcmWaveFormat.nAvgBytesPerSec, sizeof(m_pcmWaveFormat.nAvgBytesPerSec)) ;
	f->Write(&m_pcmWaveFormat.nBlockAlign, sizeof(m_pcmWaveFormat.nBlockAlign)) ;
	f->Write(&m_pcmWaveFormat.wBitsPerSample, sizeof(m_pcmWaveFormat.wBitsPerSample)) ;
	f->Write("data", 4) ;
	DWORD dwNum = m_buffer.GetNumSamples() * m_pcmWaveFormat.nBlockAlign;
	f->Write(&dwNum, sizeof(dwNum)) ;
	f->Write(m_buffer.GetBuffer(), dwNum) ;
}

void* CWave::GetBuffer() const
{
	return m_buffer.GetBuffer();
}

DWORD CWave::GetNumSamples() const
{
	return m_buffer.GetNumSamples();
}

DWORD CWave::GetBufferLength() const
{
	return ( m_buffer.GetNumSamples() * m_pcmWaveFormat.nBlockAlign );
}

void CWave::SetBuffer(void* pBuffer, DWORD dwNumSample, bool bCopy)
{
	ASSERT(pBuffer);
	ASSERT(dwNumSample > 0);
	ASSERT(m_pcmWaveFormat.nBlockAlign > 0);

	if (bCopy) {
		m_buffer.CopyBuffer(pBuffer, dwNumSample, m_pcmWaveFormat.nBlockAlign);
	}
	else {
		m_buffer.SetBuffer(pBuffer, dwNumSample, m_pcmWaveFormat.nBlockAlign);
	}
}
//////////////////////////////////////////////////////////////////////
// CWaveDevice
//////////////////////////////////////////////////////////////////////
CWaveDevice::CWaveDevice(UINT nDevice/* = WAVE_MAPPER*/) : m_nDevice(nDevice)
{
}

CWaveDevice::CWaveDevice(const CWaveDevice &copy)
{
	m_nDevice = copy.GetDevice();
}

CWaveDevice::~CWaveDevice()
{
}

bool CWaveDevice::IsInputFormat(const CWave& wave)
{
	return (waveInOpen(
		NULL,
		GetDevice(),
		&wave.GetFormat(),
		NULL,
		NULL,
		WAVE_FORMAT_QUERY) == MMSYSERR_NOERROR);
}

bool CWaveDevice::IsOutputFormat(const CWave& wave)
{
	return (waveOutOpen(
		NULL,
		GetDevice(),
		&wave.GetFormat(),
		NULL,
		NULL,
		WAVE_FORMAT_QUERY) == MMSYSERR_NOERROR);
}

inline UINT CWaveDevice::GetDevice() const
{
	return m_nDevice;
}

//////////////////////////////////////////////////////////////////////
void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	switch(uMsg) {
	case MM_WIM_DATA:
		WAVEHDR* pWaveHdr = ( (WAVEHDR*)dwParam1 );
		CWaveIn* pWaveIn = (CWaveIn*)(pWaveHdr->dwUser);

		if (pWaveHdr && hwi && pWaveIn) {
			/* FIXME */
			if ( (pWaveHdr->dwFlags & WHDR_DONE) == WHDR_DONE ) {
				pWaveHdr->dwFlags = 0;
				if ( pWaveIn->IsError(waveInUnprepareHeader(hwi, pWaveHdr, sizeof(WAVEHDR))) ) {
					break;
				}
				if (pWaveHdr->dwBytesRecorded > 0) {
					pWaveIn->AddNewBuffer(pWaveHdr);
				}
				delete[] pWaveHdr->lpData;
				pWaveHdr->lpData = NULL;
			}

			if ( !pWaveIn->ResetRequired(pWaveIn) ) {
				if ( !pWaveIn->AddNewHeader(hwi) ) {
					break;
				}
			}
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////
// CWaveIn
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CWaveIn::CWaveIn(WAVEFORMATEX tagFormat, const CWaveDevice& aDevice) : m_waveDevice(aDevice), \
	m_hWaveIn(0), m_nIndexWaveHdr(NUMWAVEINHDR - 1), m_bResetRequired(true)
{
	SetWaveFormat(tagFormat);
	InitListOfHeader();
}

CWaveIn::CWaveIn() : m_hWaveIn(0), m_bResetRequired(true)
{
	InitListOfHeader();
}

CWaveIn::~CWaveIn()
{
	Close();
	FreeListOfBuffer();
	FreeListOfHeader();
}

void CWaveIn::SetDevice(const CWaveDevice &aDevice)
{
	m_waveDevice = aDevice;
}

void CWaveIn::SetWaveFormat(WAVEFORMATEX tagFormat)
{
	m_wave.BuildFormat(tagFormat.nChannels, tagFormat.nSamplesPerSec, tagFormat.wBitsPerSample);
}

void CWaveIn::InitListOfHeader()
{
	for (int i = 0; i < NUMWAVEINHDR; i++) {
		m_tagWaveHdr[i].lpData = NULL;
	}
}

bool CWaveIn::Close()
{
	if (m_hWaveIn != NULL) {
		if ( !Stop() ) {
			return false;
		}
		if ( IsError( waveInClose(m_hWaveIn)) ) {
			return false;
		}
		m_hWaveIn = 0;
	}
	return true;
}

bool CWaveIn::Continue()
{
	if (m_hWaveIn) {
		return !IsError( waveInStart(m_hWaveIn) );
	}
	return true;
}

bool CWaveIn::Open()
{
	return !IsError( waveInOpen(&m_hWaveIn, m_waveDevice.GetDevice(), &m_wave.GetFormat(), (DWORD)waveInProc, NULL, CALLBACK_FUNCTION) );
}

bool CWaveIn::Pause()
{
	if (m_hWaveIn) {
		return !IsError( waveInStop(m_hWaveIn) );
	}
	return true;
}

bool CWaveIn::Record(UINT nTaille/* = 4096*/)
{
	ASSERT(nTaille > 0);
	ASSERT(m_hWaveIn);

	if ( !Stop() ) {
		return false;
	}
	m_bResetRequired = false;
	FreeListOfBuffer();
	FreeListOfHeader();
	SetWaveFormat( m_wave.GetFormat() );
	m_nIndexWaveHdr = NUMWAVEINHDR - 1;
	m_nBufferSize = nTaille;
	for (int i = 0; i < NUMWAVEINHDR; i++) {
		if ( !AddNewHeader(m_hWaveIn) ) {
			return false;
		}
	}
	if ( IsError(waveInStart(m_hWaveIn)) ) {
		return false;
	}

	return true;
}

bool CWaveIn::Stop()
{
	if (m_hWaveIn != NULL) {
		m_bResetRequired = true;
		::Sleep(10);
		if ( IsError(waveInReset(m_hWaveIn)) ) {
			return false;
		}
	}
	return true;
}

bool CWaveIn::AddNewBuffer(WAVEHDR *pWaveHdr)
{
	ASSERT(pWaveHdr);

	m_listOfBuffer.AddTail(new CWaveBuffer);
	( (CWaveBuffer*)m_listOfBuffer.GetTail() )->CopyBuffer( pWaveHdr->lpData, \
		pWaveHdr->dwBytesRecorded / m_wave.GetFormat().nBlockAlign, \
		m_wave.GetFormat().nBlockAlign );
	return true;
}

bool CWaveIn::AddNewHeader(HWAVEIN hwi)
{
	ASSERT(m_nBufferSize > 0);

	m_nIndexWaveHdr = (m_nIndexWaveHdr == NUMWAVEINHDR - 1) ? 0 : m_nIndexWaveHdr + 1;
	if (m_tagWaveHdr[m_nIndexWaveHdr].lpData == NULL) {
		m_tagWaveHdr[m_nIndexWaveHdr].lpData = new char[m_nBufferSize];
	}
	ZeroMemory(m_tagWaveHdr[m_nIndexWaveHdr].lpData, m_nBufferSize);
	m_tagWaveHdr[m_nIndexWaveHdr].dwBufferLength = m_nBufferSize;
	m_tagWaveHdr[m_nIndexWaveHdr].dwFlags = 0;
	m_tagWaveHdr[m_nIndexWaveHdr].dwUser = (DWORD)(void*)this;
	if ( IsError(waveInPrepareHeader(hwi, &m_tagWaveHdr[m_nIndexWaveHdr], sizeof(WAVEHDR))) ) {
		return false;
	}
	if ( IsError(waveInAddBuffer(hwi, &m_tagWaveHdr[m_nIndexWaveHdr], sizeof(WAVEHDR))) ) {
		return false;
	}
	return true;
}

void CWaveIn::FreeListOfHeader()
{
	for (int i = 0; i < NUMWAVEINHDR; i++) {
		delete[] m_tagWaveHdr[i].lpData;
		m_tagWaveHdr[i].lpData = NULL;
	}
}

void CWaveIn::FreeListOfBuffer()
{
	POSITION pos = m_listOfBuffer.GetHeadPosition();
	while (pos) {
		CWaveBuffer* pBuf = (CWaveBuffer*)m_listOfBuffer.GetNext(pos);
		if (pBuf) {
			delete pBuf;
			pBuf = NULL;
		}
	}
	m_listOfBuffer.RemoveAll();
}

DWORD CWaveIn::GetNumSamples()
{
	DWORD dwTotal = 0L;
	POSITION pos = m_listOfBuffer.GetHeadPosition();
	while (pos) {
		CWaveBuffer* p_waveBuffer = (CWaveBuffer*) m_listOfBuffer.GetNext(pos);
		dwTotal += p_waveBuffer->GetNumSamples();
	}
	return dwTotal;
}

CString CWaveIn::GetError() const
{
	if (m_nError != MMSYSERR_NOERROR) {
		TCHAR szText[MAXERRORLENGTH + 1];
		if ( waveInGetErrorText(m_nError, szText, MAXERRORLENGTH) == MMSYSERR_NOERROR ) {
			return szText;
		}
	}
	return "";
}

DWORD CWaveIn::GetPosition()
{
	if (m_hWaveIn) {
		MMTIME mmt;
		mmt.wType = TIME_SAMPLES;
		if ( IsError(waveInGetPosition(m_hWaveIn, &mmt, sizeof(MMTIME))) ) {
			return -1;
		}
		else {
			return mmt.u.sample;
		}
	}
	return -1;
}

bool CWaveIn::IsError(MMRESULT nResult)
{
	m_nError = nResult;
	return (m_nError != MMSYSERR_NOERROR);
}

bool CWaveIn::IsRecording()
{
	bool bResult = false;
	/* FIXME */
	if (m_nIndexWaveHdr > -1 && (m_tagWaveHdr[m_nIndexWaveHdr].dwFlags != 0)) {
		bResult |= !((m_tagWaveHdr[m_nIndexWaveHdr].dwFlags & WHDR_DONE )== WHDR_DONE);
	}
	return bResult;
}

CWave CWaveIn::MakeWave()
{
	void* pBuffer = new char[GetNumSamples() * m_wave.GetFormat().nBlockAlign];
	DWORD dwPosInBuffer = 0L;
	POSITION pos = m_listOfBuffer.GetHeadPosition();
	while (pos) {
		CWaveBuffer* p_waveBuffer = (CWaveBuffer*) m_listOfBuffer.GetNext(pos);
		CopyMemory( (char*)pBuffer + dwPosInBuffer, p_waveBuffer->GetBuffer(), p_waveBuffer->GetNumSamples() * p_waveBuffer->GetSampleSize() );
		dwPosInBuffer += p_waveBuffer->GetNumSamples() * p_waveBuffer->GetSampleSize();
	}
	m_wave.SetBuffer( pBuffer, GetNumSamples() );
	return m_wave;
}

bool CWaveIn::ResetRequired(CWaveIn* pWaveIn)
{
	return m_bResetRequired;
}

//////////////////////////////////////////////////////////////////////
// CWaveInterface
//////////////////////////////////////////////////////////////////////
UINT CWaveInterface::GetWaveInCount()
{
	return waveInGetNumDevs();	
}

CString CWaveInterface::GetWaveInName(UINT nIndex)
{
	ASSERT(nIndex < GetWaveInCount());
	WAVEINCAPS tagCaps;
	switch (waveInGetDevCaps(nIndex, &tagCaps, sizeof(tagCaps))) {
	case MMSYSERR_NOERROR:
		return tagCaps.szPname;
		break;
	default:
		return "";
	}
}

UINT CWaveInterface::GetWaveOutCount()
{
	return waveOutGetNumDevs();	
}

CString CWaveInterface::GetWaveOutName(UINT nIndex)
{
	ASSERT(nIndex < GetWaveOutCount());
	WAVEOUTCAPS tagCaps;
	switch (waveOutGetDevCaps(nIndex, &tagCaps, sizeof(tagCaps))) {
	case MMSYSERR_NOERROR:
		return tagCaps.szPname;
		break;
	default:
		return "";
	}
}

//////////////////////////////////////////////////////////////////////
void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	
	switch(uMsg) {
	case MM_WOM_DONE:
		WAVEHDR* pWaveHdr = ( (WAVEHDR*)dwParam1 );
		CWaveOut* pWaveOut = (CWaveOut*)(pWaveHdr->dwUser);

		if (pWaveHdr && hwo && pWaveOut) {
			if ( (pWaveHdr->dwFlags & WHDR_DONE) == WHDR_DONE) {/* FIXME */
				pWaveHdr->dwFlags = 0;
				if ( pWaveOut->IsError(waveOutUnprepareHeader(hwo, pWaveHdr, sizeof(WAVEHDR))) ) {
					break;
				}
				pWaveHdr->lpData = NULL;
			}
			if ( ! pWaveOut->ResetRequired(pWaveOut) ) {
				if ( !pWaveOut->AddNewHeader(hwo) ) {
					break;
				}
			}
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////
// CWaveOut
//////////////////////////////////////////////////////////////////////
CWaveOut::CWaveOut(const CWave& aWave, const CWaveDevice& aDevice) : m_wave(aWave), m_waveDevice(aDevice), \
	m_hWaveOut(0), m_nIndexWaveHdr(NUMWAVEOUTHDR - 1), m_dwStartPos(0L), m_dwWaveOutBufferLength(WAVEOUT_BUFFER_SIZE)
{
}

CWaveOut::CWaveOut() : m_hWaveOut(0), \
	m_dwStartPos(0L), m_dwWaveOutBufferLength(WAVEOUT_BUFFER_SIZE)
{
}

CWaveOut::~CWaveOut()
{
	Close();
}

void CWaveOut::ModifyWaveOutBufferLength(DWORD dwLength)
{
	ASSERT(dwLength > 1024);
	m_dwWaveOutBufferLength = dwLength;
}

void CWaveOut::SetDevice(const CWaveDevice &aDevice)
{
	m_waveDevice = aDevice;
}

void CWaveOut::SetWave(const CWave &aWave)
{
	m_wave = aWave;
}

bool CWaveOut::Close()
{
	if (m_hWaveOut != NULL) {
		if ( !Stop() ) {
			return false;
		}
		if ( IsError( waveOutClose(m_hWaveOut)) ) {
			return false;
		}
		m_hWaveOut = 0;
	}
	return true;
}

bool CWaveOut::Continue()
{
	if (m_hWaveOut) {
		return !IsError( waveOutRestart(m_hWaveOut) );
	}
	return true;
}

bool CWaveOut::FullPlay(int nLoop, DWORD dwStart/*=-1*/, DWORD dwEnd/*=-1*/)
{
	DWORD oldBufferLength = m_dwWaveOutBufferLength;
	m_dwWaveOutBufferLength = m_wave.GetBufferLength();

	if ( IsError(waveOutReset(m_hWaveOut)) ) {
		return false;
	}
	m_dwStartPos = (dwStart == -1) ? 0L : dwStart * m_wave.GetFormat().nBlockAlign;
	m_dwEndPos = (dwEnd == -1) ? m_wave.GetBufferLength() : __min( m_wave.GetBufferLength(), dwEnd ) * m_wave.GetFormat().nBlockAlign;
	nLoop = (nLoop == -1) ? 0 : nLoop;
	m_nIndexWaveHdr = NUMWAVEOUTHDR - 1;

	if ( !AddFullHeader(m_hWaveOut, nLoop) ) {
		m_dwWaveOutBufferLength = oldBufferLength;
		return false;
	}

	m_dwWaveOutBufferLength = oldBufferLength;

	return true;
}

bool CWaveOut::Open()
{
	return !IsError( waveOutOpen(&m_hWaveOut, m_waveDevice.GetDevice(), &m_wave.GetFormat(), (DWORD)waveOutProc, NULL, CALLBACK_FUNCTION) );
}

bool CWaveOut::Pause()
{
	if (m_hWaveOut) {
		return !IsError( waveOutPause(m_hWaveOut) );
	}
	return true;
}

bool CWaveOut::Play(DWORD dwStart/*=-1*/, DWORD dwEnd/*=-1*/)
{
	if ( !Stop() ) {
		return false;
	}
	m_dwStartPos = (dwStart == -1) ? 0L : dwStart * m_wave.GetFormat().nBlockAlign;
	m_dwEndPos = (dwEnd == -1) ? m_wave.GetBufferLength() : __min( m_wave.GetBufferLength(), dwEnd ) * m_wave.GetFormat().nBlockAlign;
	m_nIndexWaveHdr = NUMWAVEOUTHDR - 1;
	for (int i = 0; i < NUMWAVEOUTHDR; i++) {
		if ( !AddNewHeader(m_hWaveOut) ) {
			return false;
		}
	}
	return true;
}

bool CWaveOut::Stop()
{
	if (m_hWaveOut != NULL) {
		m_dwStartPos = m_dwEndPos;
		if ( IsError(waveOutReset(m_hWaveOut)) ) {
			return false;
		}
	}
	return true;
}

bool CWaveOut::AddFullHeader(HWAVEOUT hwo, int nLoop)
{
	if ( GetBufferLength() == 0) {
		return false;
	}
	m_nIndexWaveHdr = (m_nIndexWaveHdr == NUMWAVEOUTHDR - 1) ? 0 : m_nIndexWaveHdr + 1;
	m_tagWaveHdr[m_nIndexWaveHdr].lpData = (char*)m_wave.GetBuffer() + m_dwStartPos;
	m_tagWaveHdr[m_nIndexWaveHdr].dwBufferLength = m_dwEndPos - m_dwStartPos;
	m_tagWaveHdr[m_nIndexWaveHdr].dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;
	m_tagWaveHdr[m_nIndexWaveHdr].dwLoops = nLoop;
	m_tagWaveHdr[m_nIndexWaveHdr].dwUser = (DWORD)(void*)this;
	if ( IsError(waveOutPrepareHeader(hwo, &m_tagWaveHdr[m_nIndexWaveHdr], sizeof(WAVEHDR))) ) {
		return false;
	}
	if ( IsError(waveOutWrite(hwo, &m_tagWaveHdr[m_nIndexWaveHdr], sizeof(WAVEHDR))) ) {
		waveOutUnprepareHeader( hwo, &m_tagWaveHdr[m_nIndexWaveHdr], sizeof(WAVEHDR) );
		m_tagWaveHdr[m_nIndexWaveHdr].lpData = NULL;
		m_tagWaveHdr[m_nIndexWaveHdr].dwBufferLength = 0;
		m_tagWaveHdr[m_nIndexWaveHdr].dwFlags = 0;
		m_tagWaveHdr[m_nIndexWaveHdr].dwUser = NULL;
		m_nIndexWaveHdr--;
		return false;
	}
	m_dwStartPos = m_dwEndPos - m_dwStartPos;
	return true;
}

bool CWaveOut::AddNewHeader(HWAVEOUT hwo)
{
	if ( GetBufferLength() == 0) {
		return false;
	}
	m_nIndexWaveHdr = (m_nIndexWaveHdr == NUMWAVEOUTHDR - 1) ? 0 : m_nIndexWaveHdr + 1;
	m_tagWaveHdr[m_nIndexWaveHdr].lpData = (char*)m_wave.GetBuffer() + m_dwStartPos;
	m_tagWaveHdr[m_nIndexWaveHdr].dwBufferLength = GetBufferLength();
	m_tagWaveHdr[m_nIndexWaveHdr].dwFlags = 0;
	m_tagWaveHdr[m_nIndexWaveHdr].dwUser = (DWORD)(void*)this;
	if ( IsError(waveOutPrepareHeader(hwo, &m_tagWaveHdr[m_nIndexWaveHdr], sizeof(WAVEHDR))) ) {
		return false;
	}
	if ( IsError(waveOutWrite(hwo, &m_tagWaveHdr[m_nIndexWaveHdr], sizeof(WAVEHDR))) ) { 
		waveOutUnprepareHeader( hwo, &m_tagWaveHdr[m_nIndexWaveHdr], sizeof(WAVEHDR) );
		m_tagWaveHdr[m_nIndexWaveHdr].lpData = NULL;
		m_tagWaveHdr[m_nIndexWaveHdr].dwBufferLength = 0;
		m_tagWaveHdr[m_nIndexWaveHdr].dwFlags = 0;
		m_tagWaveHdr[m_nIndexWaveHdr].dwUser = NULL;
		m_nIndexWaveHdr--;
		return false;
	}
	m_dwStartPos += GetBufferLength();
	return true;
}

DWORD CWaveOut::GetBufferLength()
{
	return __min( m_dwWaveOutBufferLength, m_dwEndPos - m_dwStartPos );
}

CString CWaveOut::GetError() const
{
	if (m_nError != MMSYSERR_NOERROR) {
		TCHAR szText[MAXERRORLENGTH + 1];
		if ( waveOutGetErrorText(m_nError, szText, MAXERRORLENGTH) == MMSYSERR_NOERROR ) {
			return szText;
		}
	}
	return "";
}

DWORD CWaveOut::GetPosition()
{
	if (m_hWaveOut) {
		MMTIME mmt;
		mmt.wType = TIME_SAMPLES;
		if ( IsError(waveOutGetPosition(m_hWaveOut, &mmt, sizeof(MMTIME))) ) {
			return -1;
		}
		else {
			return mmt.u.sample;
		}
	}
	return -1;
}

bool CWaveOut::IsError(MMRESULT nResult)
{
	m_nError = nResult;
	return (m_nError != MMSYSERR_NOERROR);
}

bool CWaveOut::IsPlaying()
{
	bool bResult = false;
	if (m_nIndexWaveHdr > -1 && m_tagWaveHdr[m_nIndexWaveHdr].dwFlags != 0) {
		bResult |= !( (m_tagWaveHdr[m_nIndexWaveHdr].dwFlags & WHDR_DONE )== WHDR_DONE);/*FIXME*/
	}
	return bResult;
}

bool CWaveOut::ResetRequired(CWaveOut* pWaveOut)
{
	return (pWaveOut->m_dwStartPos >= pWaveOut->m_dwEndPos);
}

//TRY0( 200 )