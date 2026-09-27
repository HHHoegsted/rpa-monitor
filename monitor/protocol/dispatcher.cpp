#include "dispatcher.h"
#include "protocol_state_machine.h"

#include <stdexcept>


/**
 * Create a dispatcher with the supported protocol commands.
 */
ProtocolDispatcher::ProtocolDispatcher()
{
    handlers_.emplace("HELLO", handle_hello);
    handlers_.emplace("CPU", handle_cpu);
    handlers_.emplace("RAM", handle_ram);
    handlers_.emplace("DISK_FREE", handle_disk_free);
    handlers_.emplace("ROBOT_RUNNING", handle_robot_running);
    handlers_.emplace("DONE", handle_done);
    handlers_.emplace("BANANA", handle_banana);
}


/**
 * Validate and dispatch a protocol message for the current conversation state.
 *
 * Args:
 *     message: Parsed protocol message to handle.
 *     report: Machine report belonging to the current client conversation.
 *     state: Current protocol state belonging to the client conversation.
 *
 * Returns:
 *     An error response if validation fails, otherwise an OK response containing
 *     the command expected next by the protocol.
 */
std::string ProtocolDispatcher::dispatch(
    const ProtocolMessage& message,
    MachineReport& report,
    ProtocolState& state
) const
{
    if (!message.is_valid())
    {
        return "ERROR invalid message";
    }

    auto handler = handlers_.find(message.command());

    if (handler == handlers_.end())
    {
        return "ERROR unknown command";
    }

    std::optional<ProtocolState> next_state =
        ProtocolStateMachine::next_state(
            state,
            message.command()
        );

    if (!next_state.has_value())
    {
        if (state == ProtocolState::COMPLETE)
        {
            return "ERROR conversation complete";
        }

        return "ERROR unexpected command";
    }

    std::string response = handler->second(
        message,
        report
    );

    if (response.rfind("OK", 0) != 0)
    {
        return response;
    }

    state = next_state.value();

    return "OK " + ProtocolStateMachine::expected_command(state);
}


/**
 * Handle a HELLO command.
 *
 * Args:
 *     message: Parsed HELLO protocol message.
 *     report: Machine report belonging to the current client conversation.
 *
 * Returns:
 *     Protocol response for the HELLO command.
 */
std::string ProtocolDispatcher::handle_hello(
    const ProtocolMessage& message,
    MachineReport& report
)
{
    if (!report.set_hostname(message.payload()))
    {
        return "ERROR invalid hostname";
    }

    return "OK " + message.payload();
}


/**
 * Handle a CPU command.
 *
 * Args:
 *     message: Parsed CPU protocol message.
 *     report: Machine report belonging to the current client conversation.
 *
 * Returns:
 *     Protocol response for the CPU command.
 */
std::string ProtocolDispatcher::handle_cpu(
    const ProtocolMessage& message,
    MachineReport& report
)
{
    if (message.payload().empty())
    {
        return "ERROR missing CPU percentage";
    }

    try
    {
        std::size_t processed_characters = 0;

        double cpu_percent = std::stod(
            message.payload(),
            &processed_characters
        );

        if (processed_characters != message.payload().size())
        {
            return "ERROR invalid CPU percentage";
        }

        if (!report.set_cpu_percent(cpu_percent))
        {
            return "ERROR CPU percentage out of range";
        }
    }
    catch (const std::invalid_argument&)
    {
        return "ERROR invalid CPU percentage";
    }
    catch (const std::out_of_range&)
    {
        return "ERROR CPU percentage out of range";
    }

    return "OK";
}

/**
 * Handle a RAM command.
 *
 * Args:
 *     message: Parsed RAM protocol message.
 *     report: Machine report belonging to the current client conversation.
 *
 * Returns:
 *     Protocol response for the RAM command.
 */
std::string ProtocolDispatcher::handle_ram(
    const ProtocolMessage& message,
    MachineReport& report
)
{
    if (message.payload().empty())
    {
        return "ERROR missing RAM percentage";
    }

    try
    {
        std::size_t processed_characters = 0;

        double ram_percent = std::stod(
            message.payload(),
            &processed_characters
        );

        if (processed_characters != message.payload().size())
        {
            return "ERROR invalid RAM percentage";
        }

        if (!report.set_ram_percent(ram_percent))
        {
            return "ERROR RAM percentage out of range";
        }
    }
    catch (const std::invalid_argument&)
    {
        return "ERROR invalid RAM percentage";
    }
    catch (const std::out_of_range&)
    {
        return "ERROR RAM percentage out of range";
    }

    return "OK";
}

/**
 * Handle a DISK_FREE command.
 *
 * Args:
 *     message: Parsed DISK_FREE protocol message.
 *     report: Machine report belonging to the current client conversation.
 *
 * Returns:
 *     Protocol response for the DISK_FREE command.
 */
std::string ProtocolDispatcher::handle_disk_free(
    const ProtocolMessage& message,
    MachineReport& report
)
{
    if (message.payload().empty())
    {
        return "ERROR missing disk free percentage";
    }

    try
    {
        std::size_t processed_characters = 0;

        double disk_free_percent = std::stod(
            message.payload(),
            &processed_characters
        );

        if (processed_characters != message.payload().size())
        {
            return "ERROR invalid disk free percentage";
        }

        if (!report.set_disk_free_percent(disk_free_percent))
        {
            return "ERROR disk free percentage out of range";
        }
    }
    catch (const std::invalid_argument&)
    {
        return "ERROR invalid disk free percentage";
    }
    catch (const std::out_of_range&)
    {
        return "ERROR disk free percentage out of range";
    }

    return "OK";
}


/**
 * Handle a ROBOT_RUNNING protocol message.
 *
 * Args:
 *     message: Parsed protocol message containing the robot-running value.
 *     report: Machine report to update.
 *
 * Returns:
 *     Protocol response describing whether the value was accepted.
 */
std::string ProtocolDispatcher::handle_robot_running(
    const ProtocolMessage& message,
    MachineReport& report
)
{
    if (message.payload() == "true")
    {
        report.set_robot_running(true);
        return "OK";
    }

    if (message.payload() == "false")
    {
        report.set_robot_running(false);
        return "OK";
    }

    return "ERROR invalid ROBOT_RUNNING value";
}

/**
 * Handle a DONE command.
 *
 * Args:
 *     message: Parsed DONE protocol message.
 *     report: Machine report belonging to the current client conversation.
 *
 * Returns:
 *     Protocol response for the DONE command.
 */
std::string ProtocolDispatcher::handle_done(
    const ProtocolMessage& message,
    MachineReport& report
)
{
    if (!message.payload().empty())
    {
        return "ERROR unexpected DONE payload";
    }

    return "OK";
}


/**
 * Handle a BANANA command.
 *
 * Args:
 *     message: Parsed BANANA protocol message.
 *     report: Machine report belonging to the current client conversation.
 *
 * Returns:
 *     Protocol response for the BANANA command.
 */
std::string ProtocolDispatcher::handle_banana(
    const ProtocolMessage& message,
    MachineReport& report
)
{
    if (message.payload() != "42")
    {
        return "ERROR Do you even know where your towel is?";
    }

    report = MachineReport::create_diagnostic();

    return "OK TOWEL";
}