#ifndef VS_STAGE_CONTROLLER_H

#include <type_traits>
#include <initializer_list>
#include <optional>
#include <atomic>

/**
* @brief This class is used to switch between difference "stages" thread safe.
* @example:
    enum class Stages {
        Stage0,
        Stage1,
        Satge2,
        Stage3,
        Stage4,
        // ...
    };

    StageController<Stages> gStageController(Stages::Stage0);

    // from thread 1
    {
        std::Optional<Stages> prevSatge;
        if (gStageController.gotoNextStageIfCurrentStage({
                Stages::Stage1, Stages::Stage2 // We expect that the current stage to one from this set!
            }, Stages::Stage3, prevStage)) {
            // We have changed current stage from *prevSatge to Stages::Stage3 atomically
        } else {
            // We have not changed current stage, but another thread could change
        }
    }

    // from thread 2
    {
        gStageController.setNextStageStrongly(Stages::Stage4);
    }

@see real use cases in utils/ipc/msg_broker/VS_Subscriber.h
*/
namespace utils {

template<typename T>
class StageController final {
public:
    static_assert(std::is_enum_v<T> || std::is_arithmetic_v<T>);

    explicit StageController(T initStage = T());

    template<typename C>
    bool gotoNextStage(C) = delete;
    template<typename C>
    bool gotoNextStageIfCurrentStage(C prevStage, C nextStage) = delete;
    template<typename C>
    bool gotoNextStageIfCurrentStage(const std::initializer_list<C>& setOfPossibleValuesForCurrentStage, C nextStage, std::optional<C>& prevStage = {}) = delete;
    template<typename C>
    void setNextStageStrongly(C nextStage) = delete;

    bool [[nodiscard]] gotoNextStage(T nextStage);
    bool [[nodiscard]] gotoNextStageIfCurrentStage(T probableCurrentStage, T nextStage);
    bool [[nodiscard]] gotoNextStageIfCurrentStage(
        const std::initializer_list<T>& setOfPossibleValuesForCurrentStage,
        T nextStage,
        std::optional<T>& prevStage = {}
    );
    T [[nodiscard]] getCurrentStage() const;
    void setNextStageStrongly(T nextStage);

private:
    std::atomic<T> m_stage;
};

template<typename T>
StageController<T>::StageController(const T initStage) :
m_stage(initStage)
{}

template<typename T>
bool StageController<T>::gotoNextStage(const T nextStage)
{
    auto currentStage = m_stage.load();
    return m_stage.compare_exchange_strong(currentStage, nextStage);
}

template<typename T>
bool StageController<T>::gotoNextStageIfCurrentStage(T probableCurrentStage, const T nextStage)
{
    return m_stage.compare_exchange_strong(probableCurrentStage, nextStage);
}

template<typename T>
inline bool StageController<T>::gotoNextStageIfCurrentStage(
    const std::initializer_list<T>& setOfPossibleValueForCurrentStage,
    const T nextStage,
    std::optional<T>& prevStage
)
{
    for (auto possibleValueForCurrentStage : setOfPossibleValueForCurrentStage) {
        if (gotoNextStageIfCurrentStage(possibleValueForCurrentStage, nextStage)) {
            prevStage = std::optional<T>(possibleValueForCurrentStage);
            return true;
        }
    }
    return false;
}

template<typename T>
T StageController<T>::getCurrentStage() const
{
    return m_stage.load();
}

template<typename T>
inline void StageController<T>::setNextStageStrongly(const T nextStage)
{
    m_stage.store(nextStage);
}

} //! namespace utils 

#endif VS_STAGE_CONTROLLER_H
