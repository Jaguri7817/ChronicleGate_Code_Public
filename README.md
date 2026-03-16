# Chronicle Gate

## 프로젝트 소개
Chronicle Gate는 Unreal Engine 5 기반의 싱글플레이 로그라이크 액션 던전 클리어형 프로젝트입니다.
Dedicated Server 구조, Stage 기반 레벨 스트리밍, 전투 시스템, 최적화, NFT 스킨 연동을 구현했습니다.

## 공개 저장소 안내
이 저장소는 포트폴리오 검토용 공개 소스코드 리포지토리입니다.
저작권 이슈가 있을 수 있는 에셋 및 일부 콘텐츠는 제외했으며, 핵심 시스템/로직 코드만 포함했습니다.
따라서 본 저장소만으로는 프로젝트를 완전히 실행할 수 없습니다.

## 프로젝트 개요
- 엔진: Unreal Engine 5.4.4
- 언어: C++ (보조: BP)
- 장르: 로그라이크 액션 던전 클리어형 게임
- 형태: 2인 협업 프로젝트 (클라이언트 1명, 백엔드/블록체인 1명)
- 역할: Unreal Engine 클라이언트/서버 게임플레이 및 시스템 설계 및 구현

## 담당 역할
- 게임플레이/전투 시스템 구현
- Stage 기반 레벨 스트리밍 구조 설계 및 구현
- 서버/클라이언트 네트워크 처리(Replication, RPC 등) & 최적화
- Web3/NFT 연동 구조 설계 & NFT 스킨 인터페이스 구현

## 핵심 구현 포인트
### 1. Dedicated Server + Client 구조
### 2. Stage 기반 레벨 스트리밍
### 3. 서버 권위 기반 전투 처리
### 4. 스탯/HP 동기화 및 최적화
### 5. Web3/NFT 연동 & NFT 스킨 인터페이스
### 6. CPU Hitch / 네트워크 최적화

## 코드 가이드

### Dedicated Server + Client 구조
- `Source/ChronicleGateClient.Target.cs`
- `Source/ChronicleGateServer.Target.cs`
- `Source/ChronicleGateEditor.Target.cs`

Editor / Client / Dedicated Server 타겟을 분리해 프로젝트의 실행 구조를 구성한 파일입니다.

### Stage 기반 레벨 스트리밍
- `Source/ChronicleGate/Game/CGStageStreamerSystem.cpp`

다음 스테이지 선택, 비동기 로드, 텔레포트, 이전 스테이지 언로드까지의 전체 전환 흐름을 담당합니다.

### 전투 및 공격 판정
- `Source/ChronicleGate/Character/CGCharacterPlayer.cpp`

서버 권위 기반 공격 처리, 히트 판정, 데미지 적용 요청 흐름의 핵심 로직을 확인할 수 있습니다.

### 스탯/HP 동기화
- `Source/ChronicleGate/Character/CGStatComponent.*`
- `Source/ChronicleGate/Character/CGStatData.h`

캐릭터의 HP 및 기본 스탯 관리, 상태 변경 전파, UI 반영과 연결되는 중심 로직과 스탯 데이터의 직렬화(NetSerialize) 구조를 담고 있습니다.

### Web3/NFT 연동
- `Source/ChronicleGate/BlockChain/CGBackendSubsystem.cpp`

백엔드 API 통신을 통해 보상 조회, NFT 관련 데이터 수신, 게임 로직과의 연결을 담당합니다.

### NFT 스킨 인터페이스
- `Source/ChronicleGate/Equipment/CGCharacterSkinComponent.cpp`

외부에서 전달된 NFT 스킨 데이터를 게임 내 캐릭터 스킨 선택 및 적용 흐름과 연결하는 핵심 파일입니다.

## 실행 관련 안내
본 저장소는 소스코드 공개용 리포지토리이며, 저작권 이슈가 있을 수 있는 에셋 제외로 인해 전체 실행은 불가능합니다.
