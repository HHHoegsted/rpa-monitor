#include "robot_status.h"

#include <Windows.h>
#include <Wbemidl.h>
#include <wrl/client.h>

#include <optional>
#include <string>

namespace
{

constexpr wchar_t robot_command_marker[] = L"-m robocorp.tasks run";

/**
 * Determine whether a process command line belongs to a running Robocorp task.
 *
 * Args:
 *     command_line: Process command line reported by Windows.
 *
 * Returns:
 *     True when the command line contains the verified Robocorp task marker.
 */
bool is_robot_command(const BSTR command_line)
{
    if (command_line == nullptr)
    {
        return false;
    }

    const std::wstring value(
        command_line,
        SysStringLen(command_line)
    );

    return value.find(robot_command_marker) != std::wstring::npos;
}

}  // namespace

namespace robot_status
{

std::optional<bool> is_robot_running()
{
    const auto log_failure = [](const std::string& message)
    {
        HANDLE stderr_handle = GetStdHandle(STD_ERROR_HANDLE);

        if (
            stderr_handle == nullptr ||
            stderr_handle == INVALID_HANDLE_VALUE
        )
        {
            return;
        }

        DWORD characters_written = 0;

        WriteFile(
            stderr_handle,
            message.c_str(),
            static_cast<DWORD>(message.size()),
            &characters_written,
            nullptr
        );
    };

    const HRESULT com_result = CoInitializeEx(
        nullptr,
        COINIT_MULTITHREADED
    );

    const bool owns_com_initialization =
        SUCCEEDED(com_result);

    if (
        FAILED(com_result) &&
        com_result != RPC_E_CHANGED_MODE
    )
    {
        log_failure(
            "robot_status: CoInitializeEx failed.\n"
        );

        return std::nullopt;
    }

    const HRESULT security_result = CoInitializeSecurity(
        nullptr,
        -1,
        nullptr,
        nullptr,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        nullptr,
        EOAC_NONE,
        nullptr
    );

    if (
        FAILED(security_result) &&
        security_result != RPC_E_TOO_LATE
    )
    {
        log_failure(
            "robot_status: CoInitializeSecurity failed.\n"
        );

        if (owns_com_initialization)
        {
            CoUninitialize();
        }

        return std::nullopt;
    }

    const std::optional<bool> detection_result =
        [&]() -> std::optional<bool>
    {
        Microsoft::WRL::ComPtr<IWbemLocator> locator;

        HRESULT result = CoCreateInstance(
            CLSID_WbemLocator,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_IWbemLocator,
            reinterpret_cast<void**>(
                locator.GetAddressOf()
            )
        );

        if (FAILED(result))
        {
            log_failure(
                "robot_status: CoCreateInstance failed.\n"
            );

            return std::nullopt;
        }

        BSTR namespace_path = SysAllocString(
            L"ROOT\\CIMV2"
        );

        if (namespace_path == nullptr)
        {
            log_failure(
                "robot_status: SysAllocString for namespace failed.\n"
            );

            return std::nullopt;
        }

        Microsoft::WRL::ComPtr<IWbemServices> services;

        result = locator->ConnectServer(
            namespace_path,
            nullptr,
            nullptr,
            nullptr,
            0,
            nullptr,
            nullptr,
            services.GetAddressOf()
        );

        SysFreeString(namespace_path);

        if (FAILED(result))
        {
            log_failure(
                "robot_status: ConnectServer failed.\n"
            );

            return std::nullopt;
        }

        result = CoSetProxyBlanket(
            services.Get(),
            RPC_C_AUTHN_WINNT,
            RPC_C_AUTHZ_NONE,
            nullptr,
            RPC_C_AUTHN_LEVEL_CALL,
            RPC_C_IMP_LEVEL_IMPERSONATE,
            nullptr,
            EOAC_NONE
        );

        if (FAILED(result))
        {
            log_failure(
                "robot_status: CoSetProxyBlanket failed.\n"
            );

            return std::nullopt;
        }

        BSTR query_language = SysAllocString(
            L"WQL"
        );

        BSTR query = SysAllocString(
            L"SELECT CommandLine FROM Win32_Process"
        );

        if (
            query_language == nullptr ||
            query == nullptr
        )
        {
            log_failure(
                "robot_status: Failed to allocate WMI query strings.\n"
            );

            SysFreeString(query_language);
            SysFreeString(query);

            return std::nullopt;
        }

        Microsoft::WRL::ComPtr<IEnumWbemClassObject>
            enumerator;

        result = services->ExecQuery(
            query_language,
            query,
            WBEM_FLAG_FORWARD_ONLY |
                WBEM_FLAG_RETURN_IMMEDIATELY,
            nullptr,
            enumerator.GetAddressOf()
        );

        SysFreeString(query_language);
        SysFreeString(query);

        if (FAILED(result))
        {
            log_failure(
                "robot_status: ExecQuery failed.\n"
            );

            return std::nullopt;
        }

        while (true)
        {
            Microsoft::WRL::ComPtr<IWbemClassObject>
                process;

            ULONG returned = 0;

            result = enumerator->Next(
                5000,
                1,
                process.GetAddressOf(),
                &returned
            );

            if (FAILED(result))
            {
                log_failure(
                    "robot_status: Process enumeration failed.\n"
                );

                return std::nullopt;
            }

            if (returned == 0)
            {
                break;
            }

            VARIANT command_line;
            VariantInit(&command_line);

            result = process->Get(
                L"CommandLine",
                0,
                &command_line,
                nullptr,
                nullptr
            );

            if (
                SUCCEEDED(result) &&
                command_line.vt == VT_BSTR &&
                is_robot_command(
                    command_line.bstrVal
                )
            )
            {
                VariantClear(&command_line);

                return true;
            }

            VariantClear(&command_line);
        }

        return false;
    }();

    /*
     * All COM interface objects created above have now been
     * destroyed before the COM apartment is uninitialized.
     */
    if (owns_com_initialization)
    {
        CoUninitialize();
    }

    return detection_result;
}

}  // namespace robot_status