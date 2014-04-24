package name.kan.hal1.core.device;

/**
 * @author kan
 * @since 2014-04-23 00:00
 */
public interface DeviceDao
{
	long findLogicalDevice(String... physicalDeviceIds);
}
