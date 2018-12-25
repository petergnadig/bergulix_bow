select 
mdate,
imu_serial,
count(time_raw) as meresek
from m_head 
left join m_imu on fk_id_m_head=id_m_head
left join m_data on fk_id_m_imu=id_m_imu
group by 1,2

