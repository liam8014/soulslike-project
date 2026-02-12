#include "AttackWindow.h"
#include "SoulsLike/Enemy/CombatComponent.h"
#include "SoulsLike/Enemy/EnemyBase.h"

/* 애니메이션 알림(Notify)이 시작될 때 호출 (공격 판정 시작 구간) */
void UAttackWindow::NotifyBegin(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation,
                                float TotalDuration, const FAnimNotifyEventReference &EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

    // 1. 소유자(Enemy) 및 전투 컴포넌트 유효성 검사
    if (AEnemyBase *Enemy = Cast<AEnemyBase>(MeshComp->GetOwner()))
    {
        if (UCombatComponent *CombatComp = Enemy->CombatComp)
        {
            // 2. 해당 공격의 세부 속성(데미지 배율, 넉백, 방향, 소켓) 설정
            CombatComp->SetAttackAttribute(DamageMultiplier, KnockBackDistance, AttackDirection, TraceSocket);

            // 3. 공격 전조 트레이스 (플레이어의 카운터 타이밍 체크용)
            CombatComp->AttackBeforeTrace();

            // 4. 실시간 공격 스윕(판정) 활성화
            CombatComp->EnableAttackSweep();
        }
    }
}

/* 애니메이션 알림(Notify)이 종료될 때 호출 (공격 판정 종료 구간) */
void UAttackWindow::NotifyEnd(USkeletalMeshComponent *MeshComp, UAnimSequenceBase *Animation,
                              const FAnimNotifyEventReference &EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);

    // 소유자 확인 후 공격 판정 비활성화
    if (AEnemyBase *Enemy = Cast<AEnemyBase>(MeshComp->GetOwner()))
    {
        if (UCombatComponent *CombatComp = Enemy->CombatComp)
        {
            // 공격 스윕 종료
            CombatComp->DisableAttackSweep();
        }
    }
}