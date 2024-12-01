#include <ntifs.h>
#include "common.hpp"

extern "C" {
NTKERNELAPI NTSTATUS IoCreateDriver(
    PUNICODE_STRING DriverName,
    PDRIVER_INITIALIZE InitializationFunction);

NTKERNELAPI NTSTATUS MmCopyVirtualMemory(
    PEPROCESS SourceProcess,
    PVOID SourceAddress,
    PEPROCESS TargetProcess,
    PVOID TargetAddress,
    SIZE_T BufferSize,
    KPROCESSOR_MODE PreviousMode,
    PSIZE_T ReturnSize);
}

void dbgPrint(const PCSTR& text)
{
    UNREFERENCED_PARAMETER(text);
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, text));
}

namespace driver
{
    NTSTATUS create(const PDEVICE_OBJECT device_object, const PIRP irp)
    {
        UNREFERENCED_PARAMETER(device_object);
        IoCompleteRequest(irp, IO_NO_INCREMENT);
        return irp->IoStatus.Status;
    }

    NTSTATUS close(const PDEVICE_OBJECT device_object, const PIRP irp)
    {
        UNREFERENCED_PARAMETER(device_object);
        IoCompleteRequest(irp, IO_NO_INCREMENT);
        return irp->IoStatus.Status;
    }

    NTSTATUS deviceControl(const PDEVICE_OBJECT device_object, const PIRP irp)
    {
        UNREFERENCED_PARAMETER(device_object);

        dbgPrint("[+] Device control called.\n");

        const auto stackIrp{IoGetCurrentIrpStackLocation(irp)};
        if (stackIrp == nullptr)
        {
            dbgPrint("[-] IoGetCurrentIrpStackLocation(irp) is null\n");
            IoCompleteRequest(irp, IO_NO_INCREMENT);
            return STATUS_UNSUCCESSFUL;
        }

        const auto request{static_cast<Request*>(irp->AssociatedIrp.SystemBuffer)};
        if (request == nullptr)
        {
            dbgPrint("[-] request irp->AssociatedIrp.SystemBuffer is null\n");
            IoCompleteRequest(irp, IO_NO_INCREMENT);
            return STATUS_UNSUCCESSFUL;
        }

        static PEPROCESS targetProcess{nullptr};

        auto status{STATUS_UNSUCCESSFUL};

        switch (stackIrp->Parameters.DeviceIoControl.IoControlCode)
        {
        case control_codes::attach:
            status = PsLookupProcessByProcessId(request->process_id_handle, &targetProcess);
            break;
        case control_codes::read:
            if (targetProcess != nullptr)
            {
                status = MmCopyVirtualMemory(
                    targetProcess,
                    request->target_address,
                    PsGetCurrentProcess(),
                    request->buffer,
                    request->size,
                    KernelMode,
                    &request->return_size);
            }
            else
            {
                dbgPrint("[-] control_codes::read -> targetProcess is null\n");
            }
            break;
        case control_codes::write:
            if (targetProcess != nullptr)
            {
                status = MmCopyVirtualMemory(
                    PsGetCurrentProcess(),
                    request->buffer,
                    targetProcess,
                    request->target_address,
                    request->size,
                    KernelMode,
                    &request->return_size);
            }
            else
            {
                dbgPrint("[-] control_codes::write -> targetProcess is null\n");
            }
            break;
        default:
            break;
        }

        irp->IoStatus.Status = status;
        irp->IoStatus.Information = sizeof Request;

        IoCompleteRequest(irp, IO_NO_INCREMENT);

        return status;
    }
}

NTSTATUS driverMain(const PDRIVER_OBJECT driver_object, const PUNICODE_STRING registry_path)
{
    UNREFERENCED_PARAMETER(registry_path);

    dbgPrint("[+] Entered driverMain().\n");

    UNICODE_STRING device_name{};
    RtlInitUnicodeString(&device_name, L"\\Device\\xd");

    PDEVICE_OBJECT device_object{nullptr};
    NTSTATUS status{
        IoCreateDevice(
            driver_object,
            0,
            &device_name,
            FILE_DEVICE_UNKNOWN,
            FILE_DEVICE_SECURE_OPEN,
            FALSE,
            &device_object)
    };

    if (status != STATUS_SUCCESS)
    {
        dbgPrint("[-] Failed to create driver device.\n");
        return status;
    }

    dbgPrint("[+] Driver device successfully created.\n");

    UNICODE_STRING symbolic_link{};
    RtlInitUnicodeString(&symbolic_link, L"\\DosDevices\\xd");

    status = IoCreateSymbolicLink(&symbolic_link, &device_name);
    if (status != STATUS_SUCCESS)
    {
        dbgPrint("[-] Failed to establish symbolic link.\n");
        return status;
    }

    dbgPrint("[+] Driver symbolic link successfully established.\n");

    SetFlag(device_object->Flags, DO_BUFFERED_IO);

    driver_object->MajorFunction[IRP_MJ_CREATE] = driver::create;
    driver_object->MajorFunction[IRP_MJ_CLOSE] = driver::close;
    driver_object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = driver::deviceControl;

    ClearFlag(device_object->Flags, DO_DEVICE_INITIALIZING);

    dbgPrint("[+] Driver initialized successfully.\n");

    return status;
}

NTSTATUS DriverEntry()
{
    dbgPrint("[+] Entered the kernel!\n");

    UNICODE_STRING driverName{};
    RtlInitUnicodeString(&driverName, L"\\Driver\\xd");

    dbgPrint("[+] RtlInitUnicodeString\n");

    return IoCreateDriver(&driverName, &driverMain);
}
